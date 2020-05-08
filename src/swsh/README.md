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

This feature automates some aspects of Max Raid Battles, in order to get the
Pokémon you want.

When activated, you need to press the button again to select one of the three
subfeatures, described below.

#### Complete Max Raid Battle setup [Subfeature 1 - one button press]

This subfeature is the main one; it gives you the wanted Max Raid Battle from
an empty den, in a semi-automatic manner, using one Wishing Piece.

**Pre-requisites:**

 - Be in front of a Pokémon Den. It should be completely empty (no light
   pillar, no glow — if it is glowing, press A to collect the Watts and close
   the dialog box). Your character should either face the center of the den or
   be on the bicycle (but check that you can actually interact with the den in
   that case).
 - Have at least one Wishing Piece.
 - In the X Menu, the Settings button should be in the bottom right corner (its
   default position).
 - The Switch’s clock should be set to roughly the current time, and that
   current time should not be just before midnight (letting this automation
   feature running when the day changes may cause undesired behavior).
 - Be in offline play before starting (online play can cause spurious
   “Disconnected from the server” messages that will mess up the automation).

**Note:** This feature does not require a special text speed or
automatic/manual clock configuration to be set beforehand. However, it will
modify these parameters during operation, and when done, the text speed will be
set to “Fast” and the clock setting will be set to “automatic”.

Before starting, you need to select which den to use. The Pokémon that can be
found in a Max Raid Battle depend on the den and the light pillar color (red:
normal Raid Battle, bright purple: rare Raid Battle). Choose the appropriate
den, make sure the appropriate pre-requisite are met, then start the automation
feature.

The process will start by setting the text speed to slow, and save the game.
This is necessary the next step. It will then drop a Wishing Piece in the den,
and pause the game after the light pillar start forming but before the game is
saved. **Watch closely** as it happens: if the light pillar color (and thus the
Raid Battle type) is what you want, **press the pushbutton** while the game is
sitting on the home menu. If it isn’t, wait a couple of seconds; the process
will quit/restart the game and re-drop the same Wishing Piece in a loop until
you get the Max Raid Battle you want. Getting a rare Max Raid Battle (bright
purple light pillar) may take a few minutes.

Once you press the button, the process will move on to the next step; the text
speed will be set back to fast, and the Switch’s clock sync will be set to
manual. The Max Raid Battle menu will be then entered.

If the menu does not shown the Pokémon that you want, wait a few seconds; the
process will change the clock date twice to reset the Max Raid Battle. You will
also gain Watts in the process, which may be a good way to farm them.

When the Pokémon that you want is shown, press the pushbutton. The clock
settings will be restored and you will get back control. At this point, you can
safely close the Max Raid Battle menu without losing the Pokémon, since the
automation process always enter this menu with the clock set to the current
day. You can then start online play to battle the Pokémon, or save the game so
you can re-battle the Pokémon in case it escapes (but note that Pokémon in rare
Max Raid Battles — bright purple light pillar — have a 100% catch rate with any
Pokéball so you will never lose them as long as you are not out of Pokéballs).

Once you are done, you can go back to the Switch’s main menu and press the
pushbutton to get back to the automation “main menu” where you can choose
another automation task. If you restart the game without saving, the light
pillar will still be up, and you can change the Max Raid Pokémon by activating
subfeature 3 (see below for details). Note that you can only get one Pokémon
per Wishing Piece used.

#### Light pillar setup [Subfeature 2 - two button presses]

This subfeature perform the setup of the light pillar (so you can choose which
light pillar color/Raid type is created by the Wishing Piece), but does not
start the Max Raid Battle itself.

See the previous subfeature for requirements. When you get the appropriate Raid
type and push the button, you will be able to regain control. Pressing the
button again (from the Switch’s main menu) will get you back to the automation
“main menu” where you can choose another automation task.

This subfeature can be useful if you want to perform some actions after setting
up the light pillar but before entering the Max Raid Battle (for instance,
getting a Catch combo, in order to increase the likelihood of getting a shiny
Pokémon in the Max Raid Battle; this cannot be done before the light pillar
setup since it requires restarting the game).

#### Light pillar setup [Subfeature 3 - three button presses]

This subfeature changes the Pokémon available in the Max Raid battle.

**Pre-requisites:**

 - Be in front of a Pokémon Den activated using a Wishing Piece (either
   manually or by using subfeatures 1 or 2). It should not be glowing.
 - In the X Menu, the Settings button should be in the bottom right corner (its
   default position).
 - The Switch’s clock should be set to roughly the current time, and that
   current time should not be just before midnight (letting this automation
   feature running when the day changes may cause undesired behavior).
 - Be in offline play before starting (online play can cause spurious
   “Disconnected from the server” messages that will mess up the automation).

When activated, this feature will use clock changes to change the Pokémon
available in the Max Raid Battle. Press the button once the Pokémon you want
is shown; see subfeature 1 for details.

Using this feature should not alter the Catch combo status (since it does not
restart the game), so it may be possible to find shiny Pokémon in Max Raids
using the following procedure (currently untested):

1. Activate subfeature 2 to get the appropriate light pillar for the Pokémon
   you want.
2. Get a Catch combo for that Pokémon.
3. Enter the Raid. If it is not the Pokémon you want, activate subfeature 3
   to change it.
4. When you get the Pokémon you want in the Max Raid Battle menu, enter the
   battle with a very weak Pokémon (in order to lose the battle)
5. If the Pokémon is indeed shiny, after you lose the battle, save the game.
   This will allow you to get it back even if you restart the game. You can
   then catch it (possibly in online mode).
6. If the Pokémon is not shiny, after you lose the battle, enter the Max Raid
   Battle again and activate subfeature 3 again. This will change the Pokémon
   so you can retry step 4, hopefully without resetting the Catch combo.

### Auto Breeding [Feature 4 - four button presses]

This feature will automatically get Pokémon Eggs and hatch them.

**Pre-requisites:**

 - Be next to the Bridge Field Pokémon Nursery. This is the nursery that will provide the
   Eggs to be hatched.
 - **Be on your bike.**
 - Your team needs to be full, with a Pokémon with the Flame Body ability in any slot
   **except** the second one.
 - Your bike needs to be fully upgraded.
 - In the X Menu, the Settings button should be in the bottom right corner and the Map
   button should be in the bottom left corner (their default position).

You also need to known the Egg Cycle count of the Egg that are going to hatch.

This feature depends on Eggs being available at the Nursery more often that they hatch,
except for 5-cycle eggs. You may need to increase the changes of getting an Egg (by having
the Oval Charm, and/or having two Pokémon having different IDs and/or species in the
Nursery).

**Note:** This feature does not require a special text speed to be set beforehand.
However, it will modify these parameters during operation, and when done, the text speed
will be set to “Fast”.

After the feature is started, the LED will blink once per second. Press the button the
following number of times, depending on the Egg Cycle number:

| Egg Cycles | # presses | Approx. hatches / hour |
| ---------- | --------- | ---------------------- |
| 5          | 1         | 64                     |
| 10         | 2         | 60                     |
| 15         | 3         | 50                     |
| 20         | 4         | 40                     |
| 25         | 5         | 33                     |
| 30         | 6         | 30                     |
| 35         | 7         | 24                     |
| 40         | 8         | 22                     |

Once your selection is made, the buzzer will beep the number of times the button was
pressed, to confirm your selection.

At start, the following process is applied:

1. Set text speed to “fast”
2. Warp to the nearest location (which is the Bridge Field Pokémon Nursery)
3. Move around, to make sure an Egg is available.

The following steps will then be applied in a loop:

1. Warp to the nearest location (which is the Bridge Field Pokémon Nursery)
2. Get an Egg from the Nursery Helper, put it in slot 2, sending slot 2 to Box
3. Move around in circles (not too near the grass to avoid collisions)
4. Wait during the hatching animation
5. (for egg cycles = 5) move around more to give more time for an Egg to be available

The time for an Egg to hatch tends to vary a little bit, so the program will wait a bit
more than necessary before stopping moving around and waiting for the Egg to hatch. The
RX/TX LEDs light up during the wait; they should light up right after the “What?” dialog
appears.

During the Egg hatch animation (RX/TX lighted up, L blinking) you can interrupt the
process by pressing the button. The process will give you back control. Press it again
while on the Switch main menu (cursor on the game icon) to get back back into the
automation “main menu” where you can choose another automation task.

### Release Boxes [Feature 5 - five button presses]

This feature allows releasing one or multiple Boxes of Pokémon.

**Pre-requisites:**

 - The Boxes menu is open on the first Box whose Pokémon are to be released.
 - The selection modes (X/Y buttons) are normal.
 - The Boxes whose Pokémon are to be released are completely full.

When this feature is activated, it will start by moving the selection cursor
to the top left Pokémon in the Box. It will then wait for input; press the
button once to release all Pokémon in the current Box and move to the next Box
(on the right); press the button twice when you are done.

### Scan Boxes [Feature 6 - six button presses]

This feature will move the cursor along each Pokémon in Box, before moving to
the next Box, and so on.

**Pre-requisite:**  The Boxes menu is open on the first Box which will be
scanned.

When this feature is activated, it will start by moving the selection cursor
to the top left Pokémon in the Box, and then will start scanning all the Box
then moving to the next Box. Hold the button down to interrupt the process and
get back control.

This feature allows checking the stats of many Pokémon at once, which is
useful when you got many of them from Eggs.
