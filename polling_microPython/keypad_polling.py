"""
 - ``file``: keypad_polling.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class to process the keypad.
"""

from utime import ticks_us
from time_base import Time_base
from typing import List
from machine import Pin

class KeyPad:
    cnt: int            # counter to generate row sequence
    seq: int            # store the current sequence state
    ckey: int           # captured key with position coding
    dkey: int           # captured key with decimal coding
    rlsb: int           # The rows LSB position for keypad rows, the four gpios for rows must be consecutives starting in position rlsb
    clsb: int           # The cols LSB position for keypad cols, the four gpios for cols must be consecutives starting in position clsb
    en: bool            # Enable keypad processing
    dzero: bool         # Flag for double zero
    nkey: bool          # Flag that indicates that a key was pressed
    dbnc: bool          # Flag that indicates that debouncer is active      
    tb_seq: Time_base           # Periodic time base used to generate row sequence
    tb_dbnce: Time_base         # Periocic time base used to implement the key debouncer
    history: List[int] = []     # The last 10 pressed keys
    gpioC: List[Pin] = []        # The four gpios for cols
    gpioR: List[Pin] = []        # The four gpios for rows

    def __init__(self, rlsb: int, clsb: int, dbnc_time: int, en: bool):
        """
        Constructor for KeyPad.

        Parameters:
        - ``rlsb`` The rows LSB position for keypad rows, the four gpios for rows must be consecutives.
        - ``clsb`` The cols LSB position for keypad cols, the four gpios for cols must be consecutives.
        - ``dbnc_time`` Time in ms for the debouncer.
        - ``en`` Enable keypad processing.
        """
        # Initialize history buffer
        for i in range(10):
            self.history[0xff]
        self.cnt = 0x0
        self.seq = 0x8
        self.ckey = 0
        self.dkey = 0x1F
        self.rlsb = rlsb
        self.clsb = clsb
        self.en = en
        self.dzero = False
        self.nkey = False
        self.dbnc = False
        # Initialize time bases
        self.tb_seq = Time_base(2000, True)
        self.tb_dbnce = Time_base(dbnc_time*1000, False)

        # Initialize the gpios
        for i in range(4):
            self.gpioR[i] = Pin(self.rlsb + i, Pin.OUT)
            self.gpioC[i] = Pin(self.clsb + i, Pin.IN, Pin.PULL_DOWN)


    def capture(self, cols: int):
        """
        Capture the key pressed.
        """
        self.ckey = (cols >> 2) | self.seq
        self.decode()
        for i in range(9):
            self.history[9 - i] = self.history[9 - i - 1]
        self.history[0] = self.dkey
        self.nkey = True
    
    def decode(self):
        """
        Decode the key pressed.
        """
        if self.ckey == 0x88:
            self.dkey = 0x01
        elif self.ckey == 0x48:
            self.dkey = 0x02
        elif self.ckey == 0x28:
            self.dkey = 0x03
        elif self.ckey == 0x18:
            self.dkey = 0x0A
        elif self.ckey == 0x84:
            self.dkey = 0x04
        elif self.ckey ==0x44:
            self.dkey = 0x05
        elif self.ckey == 0x24:
            self.dkey = 0x06
        elif self.ckey == 0x14:
            self.dkey = 0x0B
        elif self.ckey == 0x82:
            self.dkey = 0x07
        elif self.ckey == 0x42:
            self.dkey = 0x08
        elif self.ckey == 0x22:
            self.dkey = 0x09
        elif self.ckey == 0x12:
            self.dkey = 0x0C
        elif self.ckey == 0x81:
            self.dkey = 0x0E
        elif self.ckey == 0x41:
            self.dkey = 0x00
        elif self.ckey == 0x21:
            self.dkey = 0x0F
        elif self.ckey == 0x11:
            self.dkey = 0x0D

    def gen_seq(self):
        """
        Generate the row sequence.
        """
        if not self.en:
            return
        
        self.cnt = (self.cnt + 1)%4
        self.seq = (1 << self.cnt) & 0x0000000F

        for i in range(4):
            if self.seq & (1 << i):
                self.gpioR[i].high()
            else:
                self.gpioR[i].low()

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
    
    def get_key(self) -> int:
        """
        Get the last key pressed.
        """
        return self.dkey
            

