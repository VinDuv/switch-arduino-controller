Pokémon Brilliant Diamond/Shining Pearl automation
==================================================

This module automates some tasks in Pokémon Brilliant Diamond and Shining Pearl.

Requirements
------------

You will need an Arduino UNO R3, an external Arduino programmer, and a
pushbutton inserted in the Arduino board, between pins 13 and GND (on the top
row).

You can additionally install a buzzer between pins 2 and GND.

See [the main README](../../README.md#required-hardware) for details.

Installation
------------

Use `make` to build the `usb-iface.hex` and `bdsp.hex` files. Flash
`usb-iface.hex` to the USB interface microcontroller (ATmega16U2), and
`bdsp.hex` to the main microcontroller (ATmega328P).

See the main README for the
[required software](../../README.md#required-software), the
[build procedure](../../README.md#building), and the
[programming procedure](../../README.md#programming).

Reset count
-----------

The legendary shiny hunting features count the number of time a reset is
performed. The reset count is stored in the Arduino’s non-volatile memory
(EEPROM) so it is kept even when the device is disconnected.

The reset count can be displayed on the Switch screen, or set to zero, using the
features described below.

You can also read the refresh count from a computer using the programmer and the
`tools/read_reset_count.py` script. (Requires Python 3)

**Note**: Re-flashing the main microcontroller will erase the EEPROM and lose
the reset count. I have not yet found a way to preserve the EEPROM contents or
to restore it after it is lost (`avrdude` can read the EEPROM, but writing to it
seems to do nothing)

Usage
-----

Plug the Arduino to the Switch; the L LED on the Arduino board should start
blinking rapidly, and the TX/RX LEDs should be off.

To start the automation process, start Pokémon BD/SP (if it is not already), and
put the game in the required state (which depends on the task to be automated;
see below for details).

Press Home to get to the Switch main menu (the selection should be on
the game icon) and press the pushbutton on the Arduino board. The emulated
controller will get auto-registered as controller 2, and then will access the
controller settings to register as controller 1. It will then get back to the
game, ready to control it.

Once it’s done, the Arduino L LED will blink once per second, and both the
RX and TX LEDs will be lit up. You are in the “main menu”, which allows you
to select which automation feature to perform. Press the pushbutton on the
board once to activate feature 1; twice to activate feature 2; etc.

The different automation features are described below.

### Temporary control [Feature 1 — one button press]

**Pre-requisites:** None

Pokémon BD/SP only allows one controller to be enabled while the game is
running; this features allows you to regain control of the game with your
controller.

When the feature is activated, the virtual controller will get back to the
main menu and open the controller management menu, allowing you to press A on
your controller to reconnect it. Once that’s done, you can close the menu
and get back to the game. The Arduino L LED will blink rapidly.

Once you’re ready to give back control to the virtual controller, press Home
to get back to the main menu Switch main menu (the selection should be on
the game icon) and press the pushbutton again. You will be back to the
game, ready to activate another feature.

While this feature is running, both TX and RX LEDs are off.

### Show reset count [Feature 2 - two button presses]

**Pre-requisites:** No menus open

This feature displays the [reset count](#reset-count) on the screen.

When it is activated, it will open the Mystery Gift by code function, then type
the number of resets on the keyboard so it is displayed. It will then wait for
a button press, then cancel out of the menus.

While this feature is running, the TX LED is on, the RX LED is off.

### Set reset count to zero [Feature 3 - three button presses]

**Pre-requisites:** None

This feature sets the [reset count](#reset-count) to zero.

When it is activated, the TX LED will light up and the L LED will blink. Press
the button once to perform the reset, or press it rapidly twice to cancel. In
both case, automation will return to the main menu.

While this feature is running, the RX LED is on, the TX LED is off.

### Shiny Arceus hunting [Feature 4 - four button presses]

**Pre-requisites:**

You will need the Azure Flute (available in your room after obtaining the
National Pokédex *and* completing Pokémon Legends: Arceus on the same console).

Go to the Spear Pillar room, go forward a bit and the staircase leading to
Arceus will appear.

Go to the top of the stairs, right before the platform, facing forward.
**Save**.

When activated, this feature will repetitively reset the game, and go forward
to trigger the encounter with Arceus. If it is not shiny, you can press the
button once to reset the game and try again.

You can press the button twice to get into temporary control, then go back to
the main menu. (This is not recommended to use when a shiny is found — it is
safer to unplug the Arduino in that case; the [reset count](#reset-count) is 
preserved anyway)

While this feature is running, both TX and RX LEDs are on.

