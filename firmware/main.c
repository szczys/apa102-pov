/********************************
* APA102-POV                 	*
* MIT License               	*
* Copyright 2015 - Mike Szczys  *
* http://jumptuck.com 	    	*
*				                *
********************************/

#define F_CPU 20000000

//lfuse: 0b11010111
//  This is 0xD7 for:
//  -No clock divide by 8
//  -Brown-out Detector
//  -Full-swing crystal oscillator (20Mhz)


#include <avr/io.h>
#include <util/delay.h>

#define TOTPIXELS   17
struct Pixels
{
    uint8_t brightness;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pixel;

struct Pixels pixels[TOTPIXELS];

/************** Setup bitbang LEDs ********************/
#define LED_DDR     DDRB
#define LED_PORT    PORTB     
#define LED_DATA    (1<<PB3)
#define LED_CLOCK   (1<<PB5)

/************** Setup input buttons *********************/
#define BUT_DDR     DDRD
#define BUT_PORT    PORTD
#define BUT_PIN     PIND
#define BUT_SW1     (1<<PD5)
#define BUT_SW2     (1<<PD6)
#define BUT_SW3     (1<<PD7)

/**************** Prototypes *************************************/
void POST(uint8_t pixels, uint8_t brightness);
void startFrame(void);
void endFrame(void);
void pushPixel(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b);
void shiftByte(uint8_t data);
void shiftZero(void);
void shiftOne(void);
void initBuffer(void);
void printBuffer(void);
void init_IO(void);
void init_interrupts(void);
void larsonScanner(uint8_t repeat);
void fadePixelsByHalf(void);
/**************** End Prototypes *********************************/

void POST(uint8_t pixels, uint8_t brightness) {
    //Bit Bang the LEDs to ensure they work despite hardware SPI
    
    uint8_t i = 0;
    startFrame();
    while (i++ <= (pixels/3)) {
        //Shift Red
        pushPixel(brightness, 0xFF, 0x00, 0x00);
        //Shift Green
        pushPixel(brightness, 0x00, 0xFF, 0x00);
        //Shift Blue
        pushPixel(brightness, 0x00, 0x00, 0xFF);
    }
    endFrame();
    
}

void startFrame(void) {
    //Start frame (0*32);
    shiftByte(0x00);    shiftByte(0x00);   shiftByte(0x00);   shiftByte(0x00);
}

void endFrame(void) {
    //End frame (1*32);
    shiftByte(0xFF);    shiftByte(0xFF);   shiftByte(0xFF);   shiftByte(0xFF);
}

void pushPixel(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b) {
    //Brightness only uses 5 LSB. 3 MSB must all be 1
    shiftByte(0b11100000 | brightness);    shiftByte(b);   shiftByte(g);   shiftByte(r);
}
void shiftByte(uint8_t data) {
    for (uint8_t i=0; i<8; i++) {
        if (data & (1<<(7-i))) { shiftOne(); }
        else { shiftZero(); }
    }
}

void shiftZero(void) {
    LED_PORT &= ~LED_DATA;    //low
    LED_PORT |= LED_CLOCK;                  //Raise Clock to shift in Data
    LED_PORT &= ~LED_CLOCK;                  //Clock low
}

void shiftOne(void) {
    LED_PORT |= LED_DATA;                   //Data high
    LED_PORT |= LED_CLOCK;                  //Raise Clock to shift in Data
    LED_PORT &= ~LED_CLOCK;                  //Clock low
}

void initBuffer(void) {
    for (uint8_t i=0; i<TOTPIXELS; i++) {
        pixels[i].brightness = 31;
        pixels[i].r = 0;
        pixels[i].g = 0;
        pixels[i].b = 0;
    }
}

void printBuffer(void) {
    startFrame();
    for (uint8_t i=0; i<TOTPIXELS; i++) {
        pushPixel(pixels[i].brightness, pixels[i].r, pixels[i].g, pixels[i].b);
    }
    endFrame();
}

void init_IO(void) {
    //BitBang LEDS
    LED_DDR |= LED_DATA | LED_CLOCK;
    LED_PORT &= ~(LED_DATA | LED_CLOCK);       //start low
    
    //Buttons
    BUT_DDR &= ~(BUT_SW1 | BUT_SW2 | BUT_SW3);      //Set as input
    BUT_PORT |= BUT_SW1 | BUT_SW2 | BUT_SW3;      //Enable pull-up
}

void init_interrupts(void) {

}

void fadeForever(void) {
    uint8_t fade = 0;
    while(1)
    {
        if (++fade > 31) { fade = 0; }
        startFrame();
        pushPixel(fade, 0xFF, 0x00, 0x00);
        pushPixel(fade, 0x00, 0xFF, 0x00);
        pushPixel(fade, 0x00, 0x00, 0xFF);
        endFrame();
        _delay_ms(20);
    }
}

void rainbowForever(uint8_t pixels, uint8_t brightness, uint8_t repeat) {    
    uint8_t cycleRed = 0xFF;
    uint8_t cycleGreen = 0;
    uint8_t cycleBlue = 0;
    uint8_t cycles = repeat;
    
    if (repeat == 0) { cycles = 1;}

    while(cycles > 0)
    {
        if (repeat == 0) {
            cycles = 1;
        }
        else { --cycles; }
        //down red
        
        while (cycleRed > 0) {
            cycleRed -= 1;
            cycleGreen = 0xFF - cycleRed;
        
            startFrame();
            for (uint8_t i=0; i<pixels; i++) { pushPixel(brightness, cycleRed, cycleGreen, cycleBlue); }
            endFrame();

            _delay_ms(2);
        }
        
        //down green
        while (cycleGreen > 0) {
            cycleGreen -= 1;
            cycleBlue = 0xFF - cycleGreen;
        
            startFrame();
            for (uint8_t i=0; i<pixels; i++) { pushPixel(brightness, cycleRed, cycleGreen, cycleBlue); }
            endFrame();

            _delay_ms(2);
        }

        //down blue
        while (cycleBlue > 0) {
            cycleBlue -= 1;
            cycleRed = 0xFF - cycleBlue;
        
            startFrame();
            for (uint8_t i=0; i<pixels; i++) { pushPixel(brightness, cycleRed, cycleGreen, cycleBlue); }
            endFrame();

            _delay_ms(2);
        }     
    }
}

void larsonScanner(uint8_t repeat) {
    int8_t dir = 1;    //direction
    uint8_t idx = 0;    //index
    uint8_t pixStart = 128; //pixelStartValue
    uint8_t cycles = repeat;
    
    initBuffer();
    pixels[idx].r = pixStart; //light the first pixel
    printBuffer();
    
    if (repeat == 0) {
        // 0 is code for loop forever
        cycles = 1;
    }
    
    while (cycles > 0) {
        
        idx += dir; //Increment the index
        
        //If we're not at one end or the other react accordingly
        if (idx == 0) {
            if (repeat == 0) {
                // 0 is code for loop forever
                cycles = 1;
            }
            else { --cycles; }
            dir = 1;
        }
        if (idx == (TOTPIXELS-1)) { dir = -1; }
        
        //Fade the tail before writing new pixel
        fadePixelsByHalf();
        
        //Write the new pixel
        pixels[idx].r = pixStart;
        
        //Show the changes we've made to the buffer
        printBuffer();
        
        //Delay before next loop
        _delay_ms(20);
    }
}

void fadePixelsByHalf(void) {
    for (uint8_t i=0; i<TOTPIXELS; i++) {
        pixels[i].r = pixels[i].r/2;
        pixels[i].g = pixels[i].g/2;
        pixels[i].b = pixels[i].b/2;
    }
}

int main(void)
{
    init_IO();
    init_interrupts();

    _delay_ms(200);

    POST(TOTPIXELS, 31);
    
    while (BUT_PIN & BUT_SW3) {
        ; //Loop until the button is pressed
    }

    while(1)
    {
        //fadeForever();
        rainbowForever(17, 5, 3);
        
        larsonScanner(4);
    }
}
