#ifndef ARENA_C_COMPILER_H
#define ARENA_C_COMPILER_H

#include <stdint.h>
#include <malloc.h>

#define arenaType(type) type *


#define ARENA_MAX_CAPACITY (1<<11)

typedef struct ArenaHeader
{
    size_t capacity;
    void *offset;
} ArenaHeader;

#define arenaGetHeader(x) ((ArenaHeader *)x-1)

#define arenaCreate(arena,capacity) do{\
    ArenaHeader *header=calloc(sizeof(*header)+sizeof(*arena)*(capacity),1);\
    header->capacity=capacity;\
    arena=(void *)(header+1);\
} while(0)

static void *arenaAllocFunc(void *arena,size_t size)
{
    ArenaHeader *header=arenaGetHeader(arena);

    header->offset+=size;
    if(header->offset-(void *)arena>header->capacity)
    {
        return NULL;
    }
    return header->offset-size;
}

#define arenaAlloc(arena,size) arenaAllocFunc((void *)arena,size)

#endif