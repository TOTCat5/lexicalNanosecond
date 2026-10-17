#ifndef LIST_C_COMPILER_H
#define LIST_C_COMPILER_H

#include <malloc.h>
#include <stdint.h>



typedef struct ListHeader
{
    size_t length;
    size_t capacity;
} ListHeader;

//just know,the "__qa_" and "_p_" means nothing but is to avoid collision

#define listGetHeader(list) ((ListHeader *)(list)-1)
#define listLength(list)    (listGetHeader(list)->length)
#define listCapacity(list)  (listGetHeader(list)->capacity)
#define listStart(list)     ((list)[0])
#define listEnd(list)       ((list)[listLength(list)-1])
#define listType(type)      type *

static inline uint64_t next_pow2(uint64_t x)
{
    if (x <= 1) return 1;
    return 1u << (32 - __builtin_clzll(x - 1));
}

#define listCreate(list,minimumCapacity) do{\
    uint64_t roundedCapacity=next_pow2(minimumCapacity);\
    ListHeader *__qa_header_p_=malloc(sizeof(*__qa_header_p_)+roundedCapacity*sizeof(*list));\
    __qa_header_p_->length=0;\
    __qa_header_p_->capacity=roundedCapacity;\
    list=(void *)(__qa_header_p_+1);\
} while(0)

#define listDestroy(list) do{\
    free(listGetHeader(list));\
} while(0)


#define listMemoryCheck(list) do{\
    ListHeader *__qa_header_p_=listGetHeader(list);\
    if(__qa_header_p_->length>=__qa_header_p_->capacity)\
    {\
        __qa_header_p_->capacity*=2;\
        __qa_header_p_=realloc(__qa_header_p_,sizeof(*__qa_header_p_)+__qa_header_p_->capacity*sizeof(*list));\
    }\
    list=(void *)(__qa_header_p_+1);\
} while(0);

#define listPushBack(list,item) do{\
    listMemoryCheck(list)\
    ListHeader *__qa_header_p_=listGetHeader(list);\
    (list)[__qa_header_p_->length++]=item;\
} while(0)

#define listEmplace(list,item,idx) do{\
    listMemoryCheck(list)\
    ListHeader *__qa_header_p_=listGetHeader(list);\
    memmove((list)+idx+1,list+idx,(__qa_header_p_->length-idx)*sizeof(*list));\
    [idx]=item;\
    __qa_header_p_->length++;\
} while(0)

#define listPopBack(list) do{\
    listGetHeader(list)->length=(listGetHeader(list)->length!=0)?(listGetHeader(list)->length-1):0;\
} while(0)

#define listRemoveAtIndex(list,idx) do{\
    ListHeader *__qa_header_p_=listGetHeader(list);\
    memmove(list+idx,list+idx+1,(__qa_header_p_->length-idx-1)*sizeof(*list));\
    __qa_header_p_->length--;\
} while(0)

#endif