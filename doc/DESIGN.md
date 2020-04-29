Inter-Processor Communication
=============================

This file documents how the communication between the main Arduino
microcontroller (ATmega328P, called “main µC” in this document) and the USB
interface microcontroller (ATmega16U2, called “USB µC”) is implemented.

Controller data to the Switch
-----------------------------

The controller is polled by the Switch at a constant rate of 125 Hz. This means
that 125 times a second, the USB µC needs to provide the current state of the
controller (pressed buttons, stick orientation, etc). This is sent in an
USB report message whose size is 8 bytes:
 - The two first bytes are the state of the buttons (A, B, …) as a bitfield.
   The controller emulated is the Pokken Tournament controller which has no
   Home and Capture buttons, but setting the appropriate bits in the bitfield
   is sufficient to make them recognized by the Switch.
 - The next byte is the status of the d-pad: each direction (top, top-left,
   left, …) is assigned a value, so there are 9 possible values.
 - The next two bytes are the state of the L Stick, the first is the X
   coordinate (0: leftmost, 128: center, 255: rightmost), the second is the
   Y coordinate (0: topmost, 128: center, 255: bottommost).
 - The next two bytes are the state of the R Stick.
 - The next byte is vendor specific; its purpose is unknown. Putting a 0 value
   seems to work correctly.

Even though the controller is polled 125 times a second, it does not mean that
games can react that quickly to input. Empirical testing on the Switch’s main
menu seems to indicate that pressing and releasing the left d-pad 5 times,
then pressing and releasing the right d-pad 5 times, does not reliably moves
the cursor back and forth unless the presses and releases last for 5 USB
report messages.

Thus, the USB µC will be programmed to repeat the same controller state five
times when queried by the Switch. This means that the effective state can
be changed 125 / 5 = 25 times a second. This could probably be easily changed
in the future if more frequent updates are needed.

Note that different systems may poll the controller at different rates; it is
possible for the automated actions to happen at a faster rate if the emulated
controller is plugged to a PC, for instance.

Processor synchronization
-------------------------

Every five controller reports (a “cycle”), the USB µC will update the data
it sends to the host. This means that the main µC needs to send updates at the
correct rate; it should also be able to do some computation between the
updates.

To achieve this, the USB µC receives in advance the data update for the next
cycle, and store it in a buffer. When the next cycle starts, it empties the
buffer and signals the main µC that it can start sending the next update.

With this design, the main µC execution is ahead of the USB µC execution by
at most one cycle.

Communication
-------------

The main µC and the USB µC are connected with a serial link and have embedded
serial controllers. That makes it relatively straightforward to send and
receive data between them.

The communication is bidirectional; the USB µC will indicate that it is ready
to accept data from the main µC by writing a byte on the serial line; the main
µC will then send a full data update on the link, and wait for the next “data
accept” byte.

To simplify the design, a busy-loop will be used for sending or receiving data,
instead of using interrupts:
 - On the USB µC side, a busy-loop is already used for USB handling, so
   processing incoming serial data can be done as part of the loop.
 - The main µC only checks for incoming serial data from time to time (when it
   verifies that the USB µC is ready for more data.

Exchanged data
--------------

The USB µC sends a single character to the main µC to signal that it is ready
to accept the next data update. This character is `'R'`.

In response, the main µC sends a string of 8 bytes to the USB µC. The first
7 bytes are the controller data (the eight byte of the controller data is not
sent as it is hard-coded as 0 in the USB µC code). The eight byte serves as
an end-of-data marker, to validate that no data was lost. It it also used to
change the state of the RX/TX LEDs on the Arduino board (these LEDs are
controlled by the USB µC). The valid values are:
 - `0xAC`: Both LEDs off
 - `0xAD`: TX LED on
 - `0xAE`: RX LED on
 - `0xAF`: Both LEDs on

Sequence of operations
----------------------

On the USB µC side, a 8-byte receive buffer is allocated to hold the data
received from the main µC. This buffer is initially filled with “neutral”
controller data (all buttons unpressed, sticks centered) and both LEDs off.

If a byte of data is available on the serial interface, it is added to the
receive buffer. If the buffer is full when this happens, a data error is
detected by the serial controller, or the last received byte is not what is
expected, the USB µC will enter “panic mode” (see below).

Another buffer, called the output buffer, contains the data that is actually
emitted when a poll request is received from the host (Switch or computer).

Every cycle, the USB µC checks if the receive buffer is full. If it is, the
receive buffer data is put into the output buffer and emptied, the state of
the LEDs is updated, and the “ready for more data” signal is sent to the main
µC.

The main µC is supposed to have provided a full data update at each cycle; this
ensures that the timings are predictable. That means at the start of a cycle,
if the USB µC receive buffer is not full, it will enter “panic mode”.

There is an exception to this rule, however; if the receive buffer is
completely empty at the start of a cycle and the output buffer contains neutral
controller data, the USB µC will not panic, and wait for the next cycle.

This allows the main µC to pause sending controller updates for as long as it
wants (to wait for user input, for instance), as long as it first sets the
controller to a neutral state.

The main µC uses a 8-byte transmit buffer, that is initially set to “neutral”
controller data and both LEDs off.

API calls like “set buttons” modify values in this transmit buffer, then send
it to the USB µC. The transmission is done as follows:

1. Wait for a byte to be received on the serial interface
2. Read the received byte
3. Write the transmit buffer on the serial interface

Resync/start-up detect
----------------------

It can be useful to be able to reprogram the main µC while the Arduino is
connected to the Switch (or another computer), for easier automation debugging.

Since the USB µC is unaffected by the main µC being reprogrammed, it can
continue processing USB requests from the host in a completely transparent
manner.

However, once the main µC restarts after programming (or after the Arduino’s
reset button is pressed), the two µC are in a desynchronized state:
 - If the main µC is interrupted while sending data to the USB µC, the USB µC
   will wait for data indefinitely and will not send a “ready for data” signal.
 - If the main µC is interrupted after a “ready for data” signal is sent by
   the USB µC but before it is taken into account, the signal will be lost
   since the serial controller will get reset.

This means that a special protocol is required to properly re-sync the µCs in
that situation. It is implemented in the following manner:

 - At startup, the USB µC waits 1 ms and sends a `'I'` character to the main
   µC.
 - At startup, the main µC waits to receive a `'I'` for 2 ms. If it receives
   it, it continues processing.
 - If that is not the case, it enters a loop:
   - Send a zero byte to the USB µC
   - Wait up to 5 ms to receive a `'S'` character. If not received, repeat.
 - This means that on reset from the main µC, whatever the USB µC’s state is,
   it will eventually receive a zero byte and store it at the end of its
   receive buffer (where the end-of-data marker should be). It will detect this
   situation and send a `'S'` character. to the main µC.

This means that the USB µC will either receive a `'I'` if it is starting at the
same time at the USB µC is, or a `'S'` if it was reprogramed/reset; in both
cases, it will receive a `'R'` character later when the USB µC is ready to
accept new data.

This start-up detection is useful for user code, since it allows performing
some actions only on power-up and not on main µC reset. For instance, the
Switch controller set-up process (press L/R, press A, press Home then A to
return to game) can be skipped when the main µC restart after being
reprogrammed.

Panic mode
----------

Both µCs can enter panic mode if they detect an abnormal situation. When in
panic mode, the LEDs (RX and TX for the USB µC, L for the main µC) will blink
a certain number of times depending on the type of error, wait for a little
while, then repeat.

When in panic mode, the main µC halts all processing (the panic function never
returns), but the USB µC still communicates with the host (it only sends
neutral controller data, though).
