#include <stdlib.h>
#include <stdint.h>

void *operator new(size_t size)
{
    return malloc(size);
}
