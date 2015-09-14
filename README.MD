APA102-POV
==========

This is a persistence of vision display using the APA102 RGB LED modules. I whipped this design up on the spot at Anool Mahidharia's KiCAD workshop (I hadn't relaly planned ahead to have a design ready). It is untested and definitely a work in progress.

Features:
---------

* 17 RGB Pixels (2 rows of 5x7 characters with a row in between and on either edge)
* IR receiver to issue commands as the display spins
* Hall effect sensor to know precisely where/when each revolution begins

TODO:
-----

* Figure out a method to spin this display
* Find a 5V supply for this board
* Write the code

Future Additions:
-----------------

* I broke out RX/TX pins. It would be fun to add an ESP8266 module to make it WiFi connected
* Power transfer from the base which probably means I need a second regulator circuit

License:
--------

Hardware design is by Mike Szczys and licensed [CC-BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/us/) 
