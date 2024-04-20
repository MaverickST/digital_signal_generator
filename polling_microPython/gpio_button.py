"""
 - ``file``: gpio_button.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class to process the button.
"""

from machine import Pin
from time_base import Time_base

class Button:
    """
    Class to process the button.

    It is used as a base class for the keypad -> Herency
    """
    gpioPin: Pin        # The gpio for the button
    en: bool            # Enable the button processing

    dbnc: bool          # Flag that indicates that debouncer is active
    dzero: bool         # Flag for double zero
    nkey: bool          # Flag that indicates that a key was pressed
    tb_dbnce: Time_base # Periocic time base used to implement the key debouncer

    def __init__(self, gpio: int, dbnc_time: int, en: bool):
        """
        Constructor for Button.

        Parameters:
        - ``gpio`` The gpio number for the button.
        - ``dbnc_time`` Time in us for the debouncer.
        - ``en`` Enable button processing.
        """
        # Initialize the button variables
        self.en = en
        self.dbnc = False
        self.dzero = False
        self.nkey = False

        self.gpioPin = Pin(gpio, Pin.IN, Pin.PULL_UP)
        self.tb_dbnce = Time_base(dbnc_time, False)

    def set_zflag(self):
        """
        Set the double zero flag.
        """
        self.dzero = True

    def clear_zflag(self):
        """
        Clear the double zero flag.
        """
        self.dzero = False

    def is_2nd_zero(self) -> bool:
        """
        This method returns true if a first zero was detected on keypad columns
        """
        return self.dzero



    