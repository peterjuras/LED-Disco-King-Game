# The Disco King Game

## How everything looks together:

[Awesome!](http://i.imgur.com/oKDG6p6.gif)

The Stepper plates provide the main input method for the user and have six integral parts that allow them to work properly.

* The bottom plate made of wood to connect everything
* Sponges (cut in half) to buffer the impact of stepping on them
* The top plates made of plexi glass so the LEDs can shine through
* Garden wire for the top plate electrical contacts
* Cardboard to raise the lower electrical contacts above the LED’s so they do not get  crushed
* Aluminium foil for the bottom contacts.

The plexiglass tops have the garden wire contacts on them so when they are stepped on they close the circuit with the bottom aluminum raisers. The top plates have a wire that connects back to an Arduino input pin to detect when the button is pressed. 

How to assemble the parts can be seen in the pictures below:
 
![assemble1](http://i.imgur.com/ZYklIeC.jpg) 

![assemble2](http://i.imgur.com/yTT39gb.jpg)

### The playing field: 
The playing field is made of five LED (150 LED’s on 5 meters) strips for the playing lanes and one LED strip for the life display. They are powered by a beQuiet computer power supply. The ends of the LED’s are tucked around the cardboards raisers in the stepper plates to make the plates themselves light up as well.

### How everything is connected:
All the visual parts of the game are handled by an Arduino Mega. 

![sketch](http://i.imgur.com/1FnmXYh.png)

### *Pew* *Pew*! Enabling sounds with a Raspberry PI
The Arduino prints sound and music codes as Serial messages. To receive them, a Python server needs to be set up on a computer that is connected to the Arduino.

We used a Raspberry PI in our setup, since it is small and it's able to receive power from the same power supply that powers the Arduino and LED strips.

#### Python Preqrequisites

[pygame](https://www.pygame.org/news)

#### Instructions

To start the server, simply start the python script:

```bash
$ ~/LED-Disco-King/Arduino-Raspberry-Connection/python raspberry_server.py
```
