"""
 - ``file``: signal_generator.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class to generate signals.
"""

from math import sin, pi
from utime import ticks_us
from time_base import Time_base
from typing import List

S_TO_US = 1000000
US_TO_S = 0.000001
SAMPLE = 16

class Signal:
    ss: int             # Signal State: 0: Sinusoidal, 1: Triangular, 2: Sawtooth, 3: Square
    en: bool            # Enable the signal generator
    freq: int           # Frequency in Hz
    amp: int            # Amplitude in mV
    offset: int         # Offset in mV
    value: int          # Current value in mV
    arrayV: List[int] = [0] * SAMPLE # Array of values for the signal
    cnt: int            # Counter for the signal
    tb_gen: Time_base   # Time base for the signal generator

    def __init__(self, freq: int, amp: int, offset: int, en: bool):
        """
        Constructor for signal_generator.

        Parameters:
        - ``freq`` Frequency in Hz.
        - ``amp`` Amplitude in mV.
        - ``offset`` Offset in mV.
        - ``en`` Enable the signal generator.
        """
        self.ss = 0
        self.en = en
        self.freq = freq
        self.amp = amp
        self.offset = offset
        self.value = 0
        self.cnt = 0
        self.tb_gen = Time_base(S_TO_US // (SAMPLE*freq), True)

    def gen_sin(self, t: int):
        """
        Generate a sinusoidal signal.
        """
        self.value = int(self.offset + self.amp*sin((2*pi*t)/SAMPLE))

    def gen_tri(self, t: int):
        """
        Generate a triangular signal.
        """
        if (t <= SAMPLE/2):
            self.value = int(self.offset + (4*self.amp*t)/SAMPLE - self.amp)
    
        else:
            self.value = int(self.offset - (4*self.amp*t)/SAMPLE + 3*self.amp)

    def gen_saw(self, t: int):
        """
        Generate a sawtooth signal.
        """
        self.value = int(self.offset + (2*self.amp*t)/SAMPLE - self.amp)

    def gen_sqr(self, t: int):
        """
        Generate a square signal.
        """
        if (t <= SAMPLE/2):
            self.value = int(self.offset + self.amp)
    
        else:
            self.value = int(self.offset - self.amp)

    def calculate(self):
        """
        Update the value of the signal generator according to the signal state.
        """

        if not self.en:
            return

        if self.ss == 0:
            for i in range(SAMPLE):
                self.gen_sin(i + 1)
                self.arrayV[i] = self.value - 800 # Offset of the DAC
        elif self.ss == 1:
            for i in range(SAMPLE):
                self.gen_tri(i + 1)
                self.arrayV[i] = self.value - 800 # Offset of the DAC
        elif self.ss == 2:
            for i in range(SAMPLE):
                self.gen_saw(i + 1)
                self.arrayV[i] = self.value - 800 # Offset of the DAC
        elif self.ss == 3:
            for i in range(SAMPLE):
                self.gen_sqr(i + 1)
                self.arrayV[i] = self.value - 800 # Offset of the DAC
        

    def get_value(self) -> int:
        """
        Get the current value of the signal generator.

        Returns:
        - ``value`` Current value in mV.
        """
        return self.value

    def set_ss(self, ss: int):
        """
        Set the signal state.

        Parameters:
        - ``ss`` Signal State: 0: Sinusoidal, 1: Triangular, 2: Sawtooth, 3: Square
        """
        self.ss = ss

    def set_freq(self, freq: int):
        """
        Set the frequency in Hz.

        Parameters:
        - ``freq`` Frequency in Hz.
        """
        self.freq = freq
        self.tb_gen.set_delta(S_TO_US // (SAMPLE*freq))

    def set_amp(self, amp: int):
        """
        Set the amplitude in mV.

        Parameters:
        - ``amp`` Amplitude in mV.
        """
        self.amp = amp

    def set_offset(self, offset: int):
        """
        Set the offset in mV.

        Parameters:
        - ``offset`` Offset in mV.
        """
        self.offset = offset
    
    


