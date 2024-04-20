"""
 - ``file``: keypad_polling.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class to process the keypad.
"""

from time_base import Time_base
from machine import Pin

class KeyPad:
    cols: int           # store the columns state
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
    history = []     # The last 10 pressed keys
    gpioC = []        # The four gpios for cols
    gpioR = []        # The four gpios for rows

    def __init__(self, rlsb: int, clsb: int, dbnc_time: int, en: bool):
        """
        Constructor for KeyPad.

        Parameters:
        - ``rlsb`` The rows LSB position for keypad rows, the four gpios for rows must be consecutives.
        - ``clsb`` The cols LSB position for keypad cols, the four gpios for cols must be consecutives.
        - ``dbnc_time`` Time in us for the debouncer.
        - ``en`` Enable keypad processing.
        """
        # Initialize history buffer
        for i in range(10):
            self.history.append(int(0xff))
        # Initialize the keypad variables
        self.cols = 0
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
        self.tb_dbnce = Time_base(dbnc_time, False)

        # Initialize the gpios
        for i in range(4):
            self.gpioR.append(Pin(self.rlsb + i, Pin.OUT))
            self.gpioC.append(Pin(self.clsb + i, Pin.IN, Pin.PULL_UP))


    def captureKey(self):
        """
        Capture the key pressed.
        Before this metod, it must be called the captureCols method.
        """
        # Key processing
        self.ckey = (self.cols << 4) | self.seq
        self.decode()
        # Store the key in the history buffer
        for i in range(9):
            self.history[9 - i] = self.history[9 - i - 1]
        self.history[0] = self.dkey
        # Set the new key flag
        self.nkey = True

    def captureCols(self):
        """
        Capture the columns state.
        """
        self.cols = 0
        for i in range(4):
            self.cols |= (self.gpioC[i].value() << i)
    
    def decode(self):
        """
        Decode the key pressed.
        """
        if self.ckey == 0xe8:
            self.dkey = 0x01
        elif self.ckey == 0xd8:
            self.dkey = 0x02
        elif self.ckey == 0xb8:
            self.dkey = 0x03
        elif self.ckey == 0x78:
            self.dkey = 0x0A
        elif self.ckey == 0xe4:
            self.dkey = 0x04
        elif self.ckey ==0xd4:
            self.dkey = 0x05
        elif self.ckey == 0xb4:
            self.dkey = 0x06
        elif self.ckey == 0x74:
            self.dkey = 0x0B
        elif self.ckey == 0xe2:
            self.dkey = 0x07
        elif self.ckey == 0xd2:
            self.dkey = 0x08
        elif self.ckey == 0xb2:
            self.dkey = 0x09
        elif self.ckey == 0x72:
            self.dkey = 0x0C
        elif self.ckey == 0xe1:
            self.dkey = 0x0E
        elif self.ckey == 0xd1:
            self.dkey = 0x00
        elif self.ckey == 0xb1:
            self.dkey = 0x0F
        elif self.ckey == 0x71:
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
            self.gpioR[i].value((self.seq >> i) & 0x1)


    def get_key(self) -> int:
        """
        Get the last key pressed.
        """
        return self.dkey
    
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
            

