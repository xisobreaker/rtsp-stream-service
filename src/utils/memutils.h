#ifndef MEMUTILS_H_H_H
#define MEMUTILS_H_H_H

#include <stddef.h>

void *mem_malloc(size_t size);
void  mem_free(void *ptr);

#endif