Morse Code Hint Box
===================

A little puzzle game as a weeding gift to our dear friends.

Two Arduino UNOs are involved: the first is the hint box, it has an e-paper
display to show hints and waits for a code word to be sent as Morse code over a
wire. It has a three pin connector. The second is what our friends get to
program themselves. They need to hook it up to the box and Morse the
code word. As code, by a button, however.

![Hint box after powering up](/images/box_with_hint.jpg)

[![Creative Commons BY-NC-SA](images/cc-by-nc-sa.png)](LICENSE.txt)

Build Instructions
------------------

Material required: Arduino Uno, cables, 2.54 mm 3-pin header (1x female, 1x
male), 2.54 mm 4-pin Dupont connector (1x male, 1x female)
[set with tool](https://www.amazon.de/gp/product/B07QX51F3B/),
[Waveshare E-Paper shield](https://www.waveshare.com/wiki/E-Paper_Shield),
[Waveshare 2.9inch 3-color e-inkdisplay](https://www.waveshare.com/wiki/2.9inch_e-Paper_Module_(C)),
a bit of filament and a 3D printer.

1. Print the housing, update the strings in the slicer files (.3cf). In the
   example, the lower part is a number in Morse code. You need to print with
   supports for the lid, requires a bit of work but are generally easy to
   remove.
2. Mount E-Paper shield to Arduino
3. Solder 3 wires to the shield: 1 to ground, 1 to D3 (this one is important and
   must be on this pin to support hardware interrupts), and 1 to SDA (that one
   doesn't matter too much, you can use another pin as long it's one supporting
   digital output and not colliding with the pins used by the shield). I have
   used solid core cables for this part.
4. Solder the cables to the 2.54 mm male pin header
5. Mount pin header into the holder in the box.
6. Create a connection with three cables, I have used blue (receive), white
   (send), and black (ground). The order on the Dupont header matters:
   ground, empty, white, blue. The empty one is to spare data pin 13, such that
   the builtin LED can be used. This way, I would plug the connector to the
   secondary Arduino into GND, empty on pin 13, and pins 12 and 11.
7. Mount e-paper display into lid, slide cable through hole and display slightly
   into overhang, move fully down and then slide slightly back into far side
   for full hold.
8. Screw lid on body
9. Modify [morse_hint_box.ino](morse_hint_box/morse_hint_box.ino) to your liking
   and upload to box Arduino.
10. Play with [simple_receiver.ino](simple_receiver/simple_receiver.ino) and
    [solution_full_alphabet.ino](solution_full_alphabet/solution_full_alphabet.ino)
    to send and receive.

![Hint box assembly](/images/box_assembly.jpg)

Ignore the switch in the image. I used it to enable switching the input from pin3
to SCL. This way I could also have used I²C for the challenge instead of digital
pins.

Conducting
----------

We did this in-person and the goal of the puzzle is to get another hint to
which person to go next. But obviously this could be tweaked to reveal
anything.

I asked our friends to install the Arduino IDE. They would get the second
Arduino to run their solution. They figured out that it was Morse code rather
quickly. I flashed the second Arduino with [ask_help.ino](ask_help/ask_help.ino).
Then, after plugging in, the Morse parameters and alphabet screens would come up.
The parameters are crucial to be able to properly send.

Then they were equipped with everything necessary and Morse from the bsd-games
package made it (almost too) easy to create the sequence. They initially sent
what was written on the box. Nice try, I nudged them towards the little puzzle
on the start screen. Then it was a matter of typing speed. Overall solving this
took maybe an hour. It was somewhat similar to
[solution_simple.ino](solution_simple/solution_simple.ino).

I'd be really curious for modifications that require sending and receiving. But
the receiver code is a bit more involved and requires some actual programming
(cf. [simple_receiver.ino](simple_receiver/simple_receiver.ino)). I would
consider this in particular if the one supposed to hack the box could do this on
their own time. For the in-person challenge the Morse reading code may be a
little bit too much.

Have fun!
