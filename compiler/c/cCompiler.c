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

char *poncutationTokens[]={
    " ",
    ".",
    ",",
    "\"",
    "\'",
    ";",
    ":",
    "{",
    "}",
    "\\\\"
};

// Assume str is null-terminated
void lex(LexToken **tokens,char *str,size_t strSize)
{
    LexToken *tokenList=NULL;
    listCreate(tokenList,2);

    for(size_t i=0;i<strSize;++i)
    {
        
    }
}

int main(int argc,char *argv[])
{
    FILE *outFile=fopen("compiler/out/cCompiler.asm","wb");

    fprintf(outFile,
        "global _start\n"
        "extern ExitProcess\n"
    );

    fprintf(outFile,
        "section .text\n"
        "_start:\n"
        "    sub rsp,40\n"
        "    call main\n"
        "    mov rcx,rax\n"
        "    call ExitProcess\n"
        "    add rsp,40\n"
    );

    fprintf(outFile,
        "main:\n"
        "    xor rax,rax\n"
        "    ret"
    );

}

