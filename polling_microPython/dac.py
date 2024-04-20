"""
 - ``file``: dac.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class to impress the DAC (8bits)
"""

from machine import Pin

RESOLUTION = 255        # 8 bits
DAC_RANGE = 9400        # 0 to 9.3V

class DAC:
    """
    Class to impress the DAC (8bits).
    """
    en: bool                # Enable the DAC processing
    gpioD = []   # The eight gpios for the DAC
    lsb: int                # The LSB position for the DAC, the eight gpios must be consecutives.
    digit_v: int            # The digit value for the DAC

    def __init__(self, lsb: int, en: bool):
        """
        Constructor for DAC.

        Parameters:
        - ``lsb`` The LSB position for the DAC, the eight gpios must be consecutives.
        - ``en`` Enable DAC processing.
        """
        # Initialize the DAC variables
        self.en = en
        self.lsb = lsb
        # Initialize the DAC gpios
        for i in range(8):
            self.gpioD.append(Pin(lsb + i, Pin.OUT)) 

    def set_dac(self, value: int):
        """
        Set the DAC value.

        Parameters:
        - ``value`` The value to be set in the DAC.
        """
        if not self.en:
            return

        self.digit_v = (value + 5000)*RESOLUTION//DAC_RANGE
        for i in range(8):
            self.gpioD[i].value(self.digit_v % 2)
            self.digit_v = self.digit_v // 2
