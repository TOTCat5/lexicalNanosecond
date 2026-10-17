#include "parse.h"

/*
    InterLang is bytecode Inter(midiate) Lang(uage).


    All InterLang variables are thought as placeholders for future real data with a unique file id and its type.

    a interlang file is set as such:
    header:
    |   magicNumber:u64("IntLang\0")
    |   version:InterLangVersion(u64)
    |   flags(u64):
    |   0-1  :endianess(little if 0,big if 1)
    |   2-63:reserved for future use
    globals(declarations and global labels):
    |   declaration:
    |   |   arguments types with id

*/


// order,since order matters,it's the conversion ladder e.g:bool is tinier than int16 so it's converted to the other
// also if change the order or add something,check in interLang.c to synchronize with the others

#define INTERLANG_TYPE_ENUM\
    X(InterLangTypeBool)\
\
    X(InterLangTypeInt8)\
    X(InterLangTypeUint8)\
\
    X(InterLangTypeInt16)\
    X(InterLangTypeUint16)\
\
    X(InterLangTypeInt32)\
    X(InterLangTypeUint32)\
\
    X(InterLangTypeInt64)\
    X(InterLangTypeUint64)\
\
    X(InterLangTypeFloat)\
    X(InterLangTypeDouble)\
\
    X(InterLangTypeBranch)\


typedef enum InterLangTypeEnum:uint16_t
{
    #define X(x) x,
    INTERLANG_TYPE_ENUM
    #undef X

    InterLangTypeNotAType=0xffff
} InterLangTypeEnum;

#define INTERLANG_INSTRUCTION_ENUM\
    X(InterLangInstructionNone)\
    X(InterLangInstructionAdd)\
    X(InterLangInstructionSub)\
    X(InterLangInstructionMul)\
    X(InterLangInstructionDiv)\
    X(InterLangInstructionNeg)\
    X(InterLangInstructionSet)\
    X(InterLangInstructionEqual)\
    X(InterLangInstructionGreaterThan)\
    X(InterLangInstructionLessThan)\
    X(InterLangInstructionGotoIf)\

typedef enum InterLangInstructionEnum:uint8_t
{
    #define X(x) x,
    INTERLANG_INSTRUCTION_ENUM
    #undef X
} InterLangInstructionEnum;


typedef uint64_t InterLangVersion;

typedef uint64_t InterLangVarId;


#define MAKE_INTERLANG_VERSION(version) ((InterLangVersion)(version)&0xffff)<<UINT64_C(48)
#define MAKE_INTERLANG_MAJOR(major)     ((InterLangVersion)(major)&0xffffffff)<<UINT64_C(16)
#define MAKE_INTERLANG_MINOR(minor)     ((InterLangVersion)(minor)&0xffff)

#define GET_INTERLANG_VERSION(version)  (((x)>>48)&0xffff)
#define GET_INTERLANG_MAJOR(version)    (((x)>>16)&0xffffffff)
#define GET_INTERLANG_MINOR(version)    (((x)>>08)&0xffff)


// version: 63-48 major: 47-16 minor: 15-00
#define CURRENT_INTERLANG_VERSION ((InterLangVersion)0x000100000010000)




#define MAX_INSTRUCTION_ARGS 4

void generateInterLangCode(AST_Node *tree,FILE *outputFile);

