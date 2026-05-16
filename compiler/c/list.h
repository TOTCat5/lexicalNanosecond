#ifndef LIST_C_COMPILER_H
#define LIST_C_COMPILER_H

#include <malloc.h>
#include <stdint.h>

typedef struct _d_ListHeader_
{
    size_t length;
    size_t capacity;
} _d_ListHeader_;

#define listGetHeader(list) ((_d_ListHeader_ *)(list)-1)
#define listLength(list)    (listGetHeader(list)->length)
#define listCapacity(list)  (listGetHeader(list)->capacity)
#define listType(type) type *

static inline uint64_t next_pow2(uint64_t x)
{
    if (x <= 1) return 1;
    return 1u << (32 - __builtin_clzll(x - 1));
}

#define listCreate(list,minimumCapacity) do{\
    uint64_t roundedCapacity=next_pow2(minimumCapacity);\
    _d_ListHeader_ *__qa_header_p_=malloc(sizeof(*__qa_header_p_)+roundedCapacity*sizeof(*list));\
    __qa_header_p_->length=0;\
    __qa_header_p_->capacity=roundedCapacity;\
    list=(void *)(__qa_header_p_+1);\
} while(0)


#define listMemoryCheck(list) do{\
    _d_ListHeader_ *__qa_header_p_=listGetHeader(list);\
    if(__qa_header_p_->length>=__qa_header_p_->capacity)\
    {\
        __qa_header_p_->capacity*=2;\
        __qa_header_p_=realloc(__qa_header_p_,sizeof(*__qa_header_p_)+__qa_header_p_->capacity*sizeof(*list));\
    }\
    list=(void *)(__qa_header_p_+1);\
} while(0);

#define listPushBack(list,item) do{\
    listMemoryCheck(list)\
    _d_ListHeader_ *__qa_header_p_=listGetHeader(list);\
    list[__qa_header_p_->length++]=item;\
} while(0)

#define listEmplace(list,item,idx) do{\
    listMemoryCheck(list)\
    _d_ListHeader_ *__qa_header_p_=listGetHeader(list);\
    memmove((list)+idx+1,list+idx,(__qa_header_p_->length-idx)*sizeof(*list));\
    [idx]=item;\
    __qa_header_p_->length++;\
} while(0)

#define listRemoveAtIndex(list,idx) do{\
    _d_ListHeader_ *__qa_header_p_=listGetHeader(list);\
    memmove(list+idx,list+idx+1,(__qa_header_p_->length-idx-1)*sizeof(*list));\
    __qa_header_p_->length--;\
} while(0)

#endif