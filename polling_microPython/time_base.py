"""
 - ``file``: time_base.py
 - ``Author``:  MST_CDA
 - ``Version``:  1.0
 - ``Date``:  2024-04-14
 - ``Description``: This module define a class for time_base, wich is the key for the polling strategy.
"""

from utime import ticks_us

class Time_base:
    """
    Class for time_base.
    It is the key for the polling strategy.
    """

    next: int
    delta: int
    en: bool

    def __init__(self, us: int, en: bool):
        """
        Constructor for time_base.

        Parameters:
        - ``us`` Time in microseconds.
        - ``en`` Enable the timer.
        """
        self.next = ticks_us() + us
        self.delta = us
        self.en = en
    
    def check(self):
        """
        Check if the timer has expired.

        Returns:
        - ``bool`` True if the timer has expired.
        """
        return (self.en and ticks_us() >= self.next)
    
    def update(self):
        """
        Update the next time in microseconds.
        """
        self.next = ticks_us() + self.delta

    def set_next(self):
        """
        Set the next time in microseconds.
        """
        self.next = self.next + self.delta

    def enable(self):
        """
        Enable the timer.
        """
        self.en = True

    def disable(self):
        """
        Disable the timer.
        """
        self.en = False

    def set_delta(self, us: int):
        """
        Set the time in microseconds.

        Parameters:
        - ``us`` Time in microseconds.
        """
        self.delta = us
