#include "parse.h"


#define INTER_LANG_TYPE_ENUM\
    X(INTER_LANG_TYPE_ADD)\
    X(INTER_LANG_TYPE_SUB)\
    X(INTER_LANG_TYPE_MUL)\
    X(INTER_LANG_TYPE_DIV)\
    X(INTER_LANG_TYPE_PUSH_STACK)\
    X(INTER_LANG_TYPE_POP_STACK)\
    X(INTER_LANG_TYPE_ASSIGN)\
    X(INTER_LANG_TYPE_CALL_FUNC)\
    X(INTER_LANG_TYPE_DEF_FUNC)\
    X(INTER_LANG_TYPE_RET_FUNC)\
    X(INTER_LANG_TYPE_END_FUNC)\


typedef enum InterLangTypeEnum
{
    #define X(x) x,
    INTER_LANG_TYPE_ENUM
    #undef X
} InterLangTypeEnum;


typedef struct InterLang
{
    InterLangTypeEnum e;
    union
    {
        struct
        {
            LexToken *a;
            LexToken *b;
            LexToken *result;
        } op;

        struct
        {
            LexToken *varName;
        } pushStack;

        struct
        {
            LexToken *varName;
        } popStack;

        struct
        {
            LexToken *dest;
            LexToken *src;
        } assign;

        struct
        {
            LexToken *func;
        } callFunc;
    };
} InterLang;


void generateInterLangCode(AST_Node *tree,listType(InterLang) outputList);

