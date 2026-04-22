#include <stdio.h>
#include "list.h"
#include <stdint.h>

#define LEX_TOKEN_ENUM\
    X(LEX_TOKEN_ID)\
    X(LEX_TOKEN_OP)\
    X(LEX_TOKEN_PONCTUATION)\
    X(LEX_TOKEN_KEYWORD)\
    X(LEX_TOKEN_LITTERAL)

typedef enum LexTokenEnum
{
    #define X(x) x,
    LEX_TOKEN_ENUM
    #undef X
} LexTokenEnum;

char *string_LexTokenEnum(LexTokenEnum e)
{
    switch(e)
    {
        #define X(x) case x: return #x;
        LEX_TOKEN_ENUM
        #undef X
    }
    return "string_LexTokenEnum UNDEFINED";
}


typedef struct LexToken
{
    LexTokenEnum e;

    char *tokenStart;
    char *tokenEnd;
} LexToken;

// Assume str is null-terminated
void lex(LexToken **tokens,char *str,size_t strSize)
{
    LexToken *tokenList=NULL;
    listCreate(tokenList,2);

    size_t i=0;
    // ALWAYS pass through this definition to set the loop i 
    #define SET_LEX_LOOP_I(value) do{i=value;if(i>=strSize){goto outOfLoop;}}while(0)

    while(true)
    {

        SET_I(1+i);
    }
    outOfLoop:
}

int main(int argc,char *argv[])
{
    FILE *outFile=fopen("compiler/out/cCompiler.asm","wb");

    fprintf(outFile,
        "global WinMain\n"
        "\n"
        "section .text\n"
        "WinMain:\n"
        "    xor eax, eax\n"
        "    ret"
    );
}

