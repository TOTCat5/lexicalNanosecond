#include <stdio.h>
#include <string.h>
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

    union
    {
        struct
        {
            char *tokenStart;
            char *tokenEnd;
        };

        struct
        {
            size_t idx;
        };
    };
} LexToken;

char ponctuationTokens[]={
    ' ',
    '.',
    ',',
    ';',
    ':',
    '(',
    ')',
    '{',
    '}',
    '[',
    ']',
    '=',
    '+',
    '-',
    '*',
    '/'
};





// return output size
size_t preprocess(char *str,size_t strSize)
{
    char *out=str;
    size_t outSize=0;
    size_t outI=0;

    bool isInString=false;
    bool comment=false;
    for(size_t i=0;i<strSize;++i)
    {
        if(str[i]=='\"')
        {
            bool hasBackslash=false;
            size_t backslashI=i;
            while(backslashI<=1)
            {
                if(str[backslashI-1]!='\\')
                {
                    break;
                }
                hasBackslash=!hasBackslash;
                --backslashI;
            }

            if(!hasBackslash)
            {
                isInString=!isInString;
            }
        }

        if((!strncmp(str+i,"\\",2))&&!isInString)
        {
            comment=true;
        }

        if(str[i]=='\n')
        {
            comment=false;
        }

        if(comment)
        {
            continue;
        }

        out[outI++]=str[i];
        outSize++;
    }
    

    return outSize;
}

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

