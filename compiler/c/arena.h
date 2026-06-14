#ifndef ARENA_C_COMPILER_H
#define ARENA_C_COMPILER_H

#include <stdint.h>
#include <malloc.h>

#define arenaType(type) void *


#define ARENA_MAX_CAPACITY (1<<11)

typedef struct ArenaHeader
{
    size_t capacity;
    void *offset;
} ArenaHeader;

#define arenaGetHeader(x) ((ArenaHeader *)x-1)

#define arenaCreate(arena,byteCount) do{\
    ArenaHeader *h_e_a_d_e_r=calloc(sizeof(*h_e_a_d_e_r)+(byteCount),1);\
    h_e_a_d_e_r->capacity=capacity;\
    arena=(void *)(h_e_a_d_e_r+1);\
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

#define arenaDestroy(arena) do{free(arenaGetHeader(arena));} while(0)

#endif