polargraph
==========

http://www.polargraph.co.uk/

Code and plans for Polargraph drawing machines.

A Polargraph machine is a vertical, gravity assisted, hanging-V plotter.
It is mechanically very simple, and consists of two stepper motors, two pieces of string, 
and a pen hanging in between them.  When the motors are mounted on a near-vertical
surface then they can be used to move the pen to any point on that surface.

The maths isn't hard either, basic pythagoras and that's about it.

The system has two parts: The hardware (or the server), and the software controller.
Hardware: The basic polagraph machine is an Arduino UNO (or other ATMEGA328 based board) 
loaded with the polargraph_server_a1 firmware, and with an Adafruit Motorshield on top.

This is fine for tethered drawing - that is, driving the machine with a computer connected
via USB.

The PolargraphSD is a more advanced machine based on an Arduino MEGA (or other ATMEGA2560
based board), with a Polarshield attached.  The Polarshield is a custom board that connects
a pair of stepstick stepper motor drivers, an XBee shaped wireless module socket and a 
touchscreen LCD with SD card slot.  The purpose of the PolargraphSD is to allow standalone
drawing, that is, drawings with no computer attached.  The LCD allows real-time control, 
pausing, and initiating drawing scripts that are saved on the SD card.

In addition the PolargraphSD firmware (polargraph_server_polarshield) has a couple of extra
functions that don't fit into an Arduino UNO's memory, circular pixel style, norwegian
pixel drawing direct from image file on the SD card, roving, and vector sprites.  

Any new features will only appear on the PolargraphSD.


http://www.polargraph.co.uk/ covers the development of the machine, and has links to 
where you can buy the gear to make one if you don't already have it.

There is an instructable (http://www.instructables.com/id/Polargraph-Drawing-Machine/) that 
details the construction of a simple machine.
