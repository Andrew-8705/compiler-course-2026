#include <stdio.h>
#include <stdlib.h>

extern "C" void check_null(void* ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Null pointer Exception!\n");
        exit(1);
    }
}