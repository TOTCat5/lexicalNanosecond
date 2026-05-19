#ifndef ARENA_C_COMPILER_H
#define ARENA_C_COMPILER_H

#include <stdint.h>
#include <malloc.h>

#define arenaType(type) type *


#define ARENA_MAX_CAPACITY (1<<11)

typedef struct ArenaHeader
{
    size_t capacity;
    uint64_t bitset[ARENA_MAX_CAPACITY/64];
}

#define arenaGetHeader(x) ((ArenaHeader *)x-1)

#define createArena(arena,capacity) do{\
    ArenaHeader *header=calloc(sizeof(*header)+sizeof(*arena)*(capacity),1);\
    header->capacity=capacity;
    arena=(void *)(header+1);\
} while(0)

#endif