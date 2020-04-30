Pokémon Sword/Shield automation
===============================

This module automates some tasks in Pokémon Sword and Shield.

Requirements
------------

You will need an Arduino UNO R3, an external Arduino programmer, and a
pushbutton inserted in the Arduino board, between pins 13 and GND (on the top
row).

You can additionally install a buzzer between pins 2 and GND.

See [the main README](../../README.md#required-hardware) for details.

Installation
------------

Use `make` to build the `usb-iface.hex` and `swsh.hex` files. Flash
`usb-iface.hex` to the USB interface microcontroller (ATmega16U2), and
`swsh.hex` to the main microcontroller (ATmega328P).

See the main README for the
[required software](../../README.md#required-software), the
[build procedure](../../README.md#building), and the
[programming procedure](../../README.md#programming).

Usage
-----

Plug the Arduino to the Switch; the L LED on the Arduino board should start
blinking rapidly, and the TX/RX LEDs should be off.

To start the automation process, start Pokémon Sword/Shield (if it is not
already), and put the game in the required state (which depends on the task to
be automated; see below for details).

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

Pokémon Sword/Shield only allows one controller to be enabled while the game
is running; this features allows you to regain control of the game with your
controller.

When the feature is activated, the virtual controller will get back to the
main menu and open the controller management menu, allowing you to press A on
your controller to reconnect it. Once that’s done, you can close the menu
and get back to the game. The Arduino L LED will blink rapidly.

Once you’re ready to give back control to the virtual controller, press Home
to get back to the main menu Switch main menu (the selection should be on
the game icon) and press the pushbutton again. You will be back to the
game, ready to activate another feature.

### Repeat press A [Feature 2 - two button presses]

**Pre-requisites:** Be at a spot where repetitively pressing A makes sense,
like in front of a member of the Digging Duo.

When this feature is activated, the A button will be repetitively pressed and
released. This allows getting items from the Digging Duo until you run out
of Watts.

Hold the pushbutton until the TX/RX LEDs stop blinking to exit this feature
and return to the “main menu”.

### Max Raid Farming / Watts farming [Feature 3 - three button presses]

This features allows repetitively restart a Max Raid until it gives the Pokémon you want,
using only one Wishing Piece.

**Pre-requisites:**

 - Be in front of a Pokémon Den. It should be completely empty (no light pillar, no glow
   — if it is glowing, press A to collect the Watts and close the dialog box). Your
   character should either face the center of the den or be on the bicycle (but check that
   you can actually interact with the den in that case).
 - Have at least one Wishing Piece.
 - In the X Menu, the Settings button should be in the bottom right corner (its default
   position).
 - The Switch’s clock should be set to roughly the current time, and that current time
   should not be just before midnight (letting this automation feature running when the
   day changes may cause undesired behavior).
 - Be in offline play before starting (online play can cause spurious “Disconnected from
   the server” messages that will mess up the automation).

**Note:** This feature does not require a special text speed or automatic/manual clock
configuration to be set beforehand. However, it will modify these parameters during
operation, and when done, the text speed will be set to “Fast” and the clock setting
will be set to “automatic”.

Before starting, you need to select which den to use. The Pokémon that can be found in
a Max Raid Battle depend on the den and the light pillar color (red: normal Raid Battle,
bright purple: rare Raid Battle). Choose the appropriate den, make sure the appropriate
pre-requisite are met, then start the automation feature.

The process will start by setting the text speed to slow, and save the game. This is
necessary the next step. It will then drop a Wishing Piece in the den, and pause the game
after the light pillar start forming but before the game is saved. **Watch closely** as it
happens: if the light pillar color (and thus the Raid Battle type) is what you want,
**press the pushbutton** while the game is sitting on the home menu. If it isn’t, wait
a couple of seconds; the process will quit/restart the game and re-drop the same Wishing
Piece in a loop until you get the Max Raid Battle you want. Getting a rare Max Raid Battle
(bright purple light pillar) may take a few minutes.

Once you press the button, the process will move on to the next step; the text speed will
be set back to fast, and the Switch’s clock sync will be set to manual. The Max Raid
Battle menu will be then entered.

If the menu does not shown the Pokémon that you want, wait a few seconds; the process will
change the clock date twice to reset the Max Raid Battle. You will also gain Watts in the
process, which may be a good way to farm them.

When the Pokémon that you want is shown, press the pushbutton. The clock settings will
be restored and you will get back control. At this point, you can safely close the Max
Raid Battle menu without losing the Pokémon, since the automation process always enter
this menu with the clock set to the current day. You can then start online play (but
go back to offline play before continuing the process) to battle the Pokémon, or save the
game so you can re-battle the Pokémon in case it escapes (but note that Pokémon in rare
Max Raid Battles — bright purple light pillar — have a 100% catch rate with any Pokéball
so you will never lose them as long as you are not out of Pokéballs).

Once you are done, you can go back to the Switch’s main menu and press the pushbutton to
re-enable the process. You can either press it once, or twice:

 - If you press it once, you will be back into the automation “main menu” where you can
   choose another automation task.
 - If you press it twice, the game will be closed, restarted, and the process will
   continue changing the Pokémon in the Max Raid Battle. **This will only work if you have
   not saved the game in the meantime, and the Pokémon you had selected will be lost.**
   This allows getting another Pokémon using the same Wishing Piece, if the one you had
   previously was not satisfying.

TODO: Implement a process that will actually start the Max Raid Battle so you can see
if the Pokémon is shiny, and either restart the game to retry, or gain back control to
battle it.
