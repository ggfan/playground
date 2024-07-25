#import shlex
import sys
import math

import ctypes

class Vector:

    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y

    def __repr__(self):
        return f'Vector({self.x!r}, {self.y!r})'

    def __abs__(self):
        return math.hypot(self.x, self.y)

    def __bool__(self):
        return bool(abs(self))

    def __add__(self, other):
        x = self.x + other.x
        y = self.y + other.y
        return Vector(x, y)

    def __mul__(self, scalar):
        return Vector(self.x * scalar, self.y * scalar)
    

def main() -> int:
    c_lib = ctypes.CDLL('./cmodule/example.so')
    
    c_lib.print.argtypes = [ctypes.c_char_p]
    c_lib.print(ctypes.c_char_p("=====starting c calls".encode()))

    c_lib.get_ver.argtypes = []
    c_lib.get_ver.restype = ctypes.c_int
    ver = c_lib.get_ver()
    print(f"c_lib ver: {ver}")  # print from python side

    c_lib.math.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_char]
    c_lib.math.restype = ctypes.c_int
    result = c_lib.math(7, 202400, b'+')

    result_str = f"result from c_lib.add(): {result}"
    c_lib.print(ctypes.c_char_p(result_str.encode()))

    # calling something not existing...
    # c_lib.hello()

    c_lib.print(ctypes.c_char_p("====c_calls ends.".encode()))

    # ===== python codes starts
    colors = ['black', 'white']
    sizes  = ['M', 'L', 'S']

    all_combs = [(size, color) for size in sizes \
                               for color in colors]
    
    print(all_combs)

    #gen expression
    for tshirt in (f'{size} {color}' for size in sizes \
                                     for color in colors):
        print(tshirt)

    return 0

if __name__ == '__main__':
    sys.exit(main())