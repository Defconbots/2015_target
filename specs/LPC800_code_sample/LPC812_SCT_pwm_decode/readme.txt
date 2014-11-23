Purpose:
SCT program that implements a PWM decoder (a pulse width measurement).
The signal frequency is assumed to be 10 kHz. 
Indicator output for missing signal.
Indicator output for "duty cycle < min  ||  duty cycle > max"

An an inverted pulse can easily be used by using the input pin inverter logic!

Running mode:
* Compile, Flash the program and reset.
* Default project target set to Blinky_Release (executing from flash)

Note:
Tested on LPC800 LPCXpresso Board
LPC800 running at 24 MHz

Input:
PIO0_14 configured as CTIN_0 (apply the 10 kHz PWM signal here)

Output:
PIO0_17 configured as CTOUT_0
        Timeout indicator, low active.
        Is active if no signal edge detected for three 10 kHz periods.
PIO0_7 configured as CTOUT_1
        Indicator for duty cycle out of bounds, low active.
        Also active when timeout occurs.

 