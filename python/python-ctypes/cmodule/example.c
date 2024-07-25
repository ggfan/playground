#include <stdio.h>
static const int version = 0x00000001;

/*
    Usage: c_lib = ctypes.CDLL('./cmodule/example.so')

    c_lib.get_ver.argtypes = []
    c_lib.get_ver.restype = ctypes.c_int
    ver = c_lib.get_ver()
*/
int get_ver(void) {
    return version;
}

/**
 * Usage:
 *      c_lib.math.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_char]
 *      c_lib.math.restype = ctypes.c_int
 *      result = c_lib.math(7, 202400, b'+')
 */
int math(int a, int b, char op) {
    int res = 0;
    switch (op) {
    case '+': res = a + b; break;
    case '-': res = a - b; break;
    case '*': res = a * b; break;
    case '/': res = (int)((float)a / b + 0.5f); break;
    default:
        res = 0;
        printf("wrong operator: %c\n", op);
    }

    return res;
}

/**
 * Usage:
 *      c_lib.print(ctypes.c_char_p("starting c calls".encode()))
 */
void print(const char* str) {
    printf("%s\n", str);
}
