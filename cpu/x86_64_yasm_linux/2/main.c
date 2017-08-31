#include <stdio.h>

/*
 * declaring add as extern tells the compiler that the definition
 * can be found in a separate modules
 */
extern int _add(int a, int b);

int main(int agrc, char** argv) {
   
    int ret;
    printf("=== starting...\n");
    ret =  _add (10, 20 );
    printf("add returned %d\n", ret);
    return 0;
}

