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
            Pin(self.rlsb + i, Pin.IN, Pin.PULL_DOWN)
            Pin(self.clsb + i, Pin.OUT, Pin.PULL_DOWN)

        def capture(self):
            """
            Capture the key pressed.
            """
            if self.nkey:
                self.history.pop(0)
                self.history.append(self.dkey)
                self.nkey = False
                self.dbnc = True
                self.tb_dbnce.reset()
            if self.dbnc and self.tb_dbnce.state:
                self.dbnc = False
                self.ckey = 0x0
                self.dkey = 0x1F
            if self.ckey != 0x0:
                self.nkey = True
            else:
                self.nkey = False
                self.dbnc = False
        
        def decode(self):
            """
            Decode the key pressed.
            """
            for i in range(4):
                Pin(self.clsb + i, Pin.OUT, Pin.PULL_UP)
                for j in range(4):
                    if Pin(self.rlsb + j).value() == 1:
                        self.ckey = (i << 2) + j
                        self.dkey = (i << 2) + j + 1
                Pin(self.clsb + i, Pin.OUT, Pin.PULL_DOWN)

