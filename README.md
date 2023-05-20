# Powerwindowcontrolsystem-using-Tiva-C-running-FreeRTOS

Project scope

1. Implementation of front passenger door window with both passenger and driver control
panels.
2. FreeRTOS implementation is a must.
3. Implementation of 2 limit switches to limit the window motor from top and bottom limits of
the window.
4. Obstacle detection implementation is required, no need for current stall sensor, just use a
push button to indicate jamming.
Shown in the figure areas of the car to be implemented highlighted in red.

System basic features

1. Manual open/close function
When the power window switch is pushed or pulled
continuously, the window opens or closes until the switch
is released.
2. One touch auto open/close function
When the power window switch is pushed or pulled
shortly, the window fully opens or closes.
3. Window lock function
When the window lock switch is turned on, the opening and closing of
all windows except the driver’s window is disabled.
4. Jam protection function
This function automatically stops the power window and moves it
downward about 0.5 second if foreign matter gets caught in the
window during one touch auto close operation.
