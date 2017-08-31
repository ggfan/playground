#include <stdio.h>
#include <inttypes.h>
/*
 * declaring add as extern tells the compiler that the definition
 * can be found in a separate modules
 */
extern int64_t _add(int64_t a, int64_t b);

int main(int agrc, char** argv) {
   
    int64_t ret;
    printf("=== starting...\n");
    ret =  _add ((int64_t)15, (int64_t)20);
    printf("add returned %" PRId64 "\n", ret);
    return 0;
}

