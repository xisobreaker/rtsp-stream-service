#include "memutils.h"

#include <stdlib.h>
#include <string.h>

void *mem_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void mem_free(void *ptr)
{
    free(ptr);
}
