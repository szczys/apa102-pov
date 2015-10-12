/********************************
* APA102-POV                 	*
* MIT License               	*
* Copyright 2015 - Mike Szczys  *
* http://jumptuck.com 	    	*
*				                *
********************************/

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>


/************** Setup bitbang LEDs ********************/
#define LED_DDR     DDRB
#define LED_PORT    PORTB     
#define LED_DATA    (1<<PB3)
#define LED_CLOCK   (1<<PB5)

/************** Setup input buttons *********************/
#define BUT_DDR     DDRD
#define BUT_PORT    PORTD
#define BUT_PIN     PINd
#define BUT_SW1    (1<<PD5)
#define BUT_SW2     (1<<PD6)
#define BUT_SW3     (1<<PD7)

/**************** Prototypes *************************************/
void POST(void);
void shiftByte(uint8_t data);
void shiftZero(void);
void shiftOne(void);
void init_IO(void);
void init_interrupts(void);
/**************** End Prototypes *********************************/

void POST(void) {
    //Bit Bang the LEDs to ensure they work despite hardware SPI
    
    //Start fram (0*32);
    shiftByte(0x00);    shiftByte(0x00);   shiftByte(0x00);   shiftByte(0x00);
    
    //Shift Red
    shiftByte(0xFF);    shiftByte(0x00);   shiftByte(0x00);   shiftByte(0xFF);
    //Shift Green
    shiftByte(0xFF);    shiftByte(0x00);   shiftByte(0xFF);   shiftByte(0x00);
    //Shift Blue
    shiftByte(0xFF);    shiftByte(0xFF);   shiftByte(0x00);   shiftByte(0x00);
    
    //End frame (1*32);
    shiftByte(0xFF);    shiftByte(0xFF);   shiftByte(0xFF);   shiftByte(0xFF);
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

int main(void)
{
    init_IO();
    init_interrupts();

    _delay_ms(200);

    POST();

    while(1)
    {

    }
}
