"""
 - ``file``: gpio_led.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class to process a led.
"""

from machine import Pin

class Led:
    """
    Class to process a led.
    """
    gpioPin: Pin       # The gpio for the led
    on: bool           # Its principal aim is to make a toggle

    def __init__(self, gpio: int, on: bool):
        """
        Constructor for Led.

        Parameters:
        - ``gpio`` The gpio number for the led.
        - ``state`` True to turn on the led, False to turn off the led.
        """
        # Initialize the led variables
        self.on = on
        self.gpioPin = Pin(gpio, Pin.OUT)
        self.gpioPin.value(self.on)

    def set(self):
        """
        Turn on the led.
        """
        self.gpioPin.high()
        self.on = True

    def clear(self):
        """
        Turn off the led.
        """
        self.gpioPin.low()
        self.on = False

    def toggle(self):
        """
        Toggle the led.
        """
        self.gpioPin.value(not self.on)
        self.on = not self.on