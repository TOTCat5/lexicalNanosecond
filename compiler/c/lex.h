#pragma once

#include <stdio.h>
#include <string.h>
#include "list.h"
#include "arena.h"
#include <stdint.h>




bool toggleString(char *str,size_t i,size_t strSize);

// return output size
size_t preprocess(char *str,size_t strSize);



#define LEX_TOKEN_ENUM\
    X(LEX_TOKEN_UNDEFINED)\
    X(LEX_TOKEN_ID)\
    X(LEX_TOKEN_PONCTUATION)\
    X(LEX_TOKEN_CONSTANT)\
    X(LEX_TOKEN_IMPORT)\
    X(LEX_TOKEN_FOR)\
    X(LEX_TOKEN_FROM)\
    X(LEX_TOKEN_TO)\
    X(LEX_TOKEN_UNTIL)\
    X(LEX_TOKEN_WHILE)\
    X(LEX_TOKEN_IF)\
    X(LEX_TOKEN_RETURN)\
    X(LEX_TOKEN_AS)\
    X(LEX_TOKEN_STRUCT)\


typedef enum LexTokenEnum
{
    #define X(x) x,
    LEX_TOKEN_ENUM
    #undef X
} LexTokenEnum;

const char *string_LexTokenEnum(LexTokenEnum e);

typedef struct LexToken
{
    LexTokenEnum e;

    union
    {
        char ponctuation; 

        struct
        {
            char *str;
            size_t strLen;
        };
    };
} LexToken;

#define isTokenPonc(token,ponc) (((token).e==LEX_TOKEN_PONCTUATION)&&((token).ponctuation==ponc)) 


static void printLexToken(FILE *file,const LexToken *token)
{
    switch(token->e)
    {
        case LEX_TOKEN_PONCTUATION:
            fprintf(file,"%c",token->ponctuation);
        break;

        default:
            fprintf(file,"%.*s",token->strLen,token->str);
        break;
    }
}



#define NO_TOKEN SIZE_MAX

#define KEYWORDS_LIST\
    X("import")\
    X("for")\
    X("from")\
    X("to")\
    X("until")\
    X("while")\
    X("if")\
    X("return")\
    X("as")\
    X("struct")\


// Assume str is null-terminated
void lex(listType(LexToken) *pTokens,char *str,size_t strSize);
