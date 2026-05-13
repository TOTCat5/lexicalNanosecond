#include <stdio.h>
#include <string.h>
#include "list.h"
#include <stdint.h>




bool toggleString(char *str,size_t i,size_t strSize)
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

        if(hasBackslash)
        {
            return true;
        }
    }

    return false;
}

// return output size
size_t preprocess(char *str,size_t strSize)
{
    char *out=str;
    size_t outSize=0;
    size_t outI=0;

    bool isInString=false;
    bool comment=false;

    // Comments Removal Pass
    for(size_t i=0;i<strSize;++i)
    {
        isInString^=toggleString(str,i,strSize);

        if((!strncmp(str+i,"\\\\",2))&&!isInString)
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
        ++outSize;
    }
    out[outI]='\0';

    // Reset and preparring Next Pass
    size_t commentAndSpacesPassSize=outSize;
    outSize=0;
    outI=0;
    char stuffToRemove[]={
        '\n',
        '\r',
        '\t'
    };
    // Useless One Character-Wide Stuff Removal Pass
    for(size_t i=0;i<commentAndSpacesPassSize;++i)
    {
        isInString^=toggleString(str,i,commentAndSpacesPassSize);

        
        if(isInString)
        {
            goto afterChecks;
        }

        bool found=false;
        for(size_t j=0;j<sizeof(stuffToRemove)/sizeof(stuffToRemove[0]);++j)
        {
            if(str[i]==stuffToRemove[j])
            {
                found=true;
                break;
            }
        }
        if(found)
        {
            continue;
        }

        afterChecks:

        out[outI++]=str[i];
        ++outSize;
    }

    // Reset For Next Pass
    size_t oneWideChacterRemovalSize=outSize;
    outSize=0;
    outI=0;
    
    bool wasSpace=false;
    // Useless Following Spaces Removal Pass
    for(size_t i=0;i<oneWideChacterRemovalSize;++i)
    {
        isInString^=toggleString(str,i,oneWideChacterRemovalSize);

        if(str[i]==' '&&!isInString)
        {
            if(wasSpace)
            {
                continue;
            }
        }
        wasSpace=str[i]==' ';
        

        out[outI++]=str[i];
        ++outSize;
    }

    out[outI]='\0';

    return outSize;
}



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
    X(KEX_TOKEN_RETURN)


typedef enum LexTokenEnum
{
    #define X(x) x,
    LEX_TOKEN_ENUM
    #undef X
} LexTokenEnum;

const char *string_LexTokenEnum(LexTokenEnum e)
{
    switch(e)
    {
        #define X(x) case x: return #x;
        LEX_TOKEN_ENUM
        #undef X
    }
    return __func__;
}

typedef struct LexToken
{
    LexTokenEnum e;

    union
    {
        struct
        {
            char c;
        } ponctuation;

        struct
        {
            char *str;
            size_t strLen;
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
    '/',
    '!'
};

char *keywords[]={
    "import",
    "for",
    "from",
    "to",
    "until",
    "if",
    "return"
};

#define NO_TOKEN SIZE_MAX
size_t checkForPonctuationToken(char token)
{
    size_t foundIdx=SIZE_MAX;
    for(size_t j=0;j<sizeof(ponctuationTokens)/sizeof(ponctuationTokens[0]);++j)
    {
        if(token==ponctuationTokens[j])
        {
            foundIdx=j;
        }
    }

    return foundIdx;
}




// Assume str is null-terminated
void lex(listType(LexToken) *pTokens,char *str,size_t strSize)
{
    LexToken *tokenList=NULL;
    listCreate(tokenList,2);

    size_t startStrI=-1;

    for(size_t i=0;i<strSize;++i)
    {
        
        // If i get to reach this one I swear to god I delete this project
        size_t foundIdx=checkForPonctuationToken(str[i]);
        if(foundIdx!=NO_TOKEN)
        {
            if(startStrI!=i-1)
            {
                LexToken token={
                    .e=LEX_TOKEN_UNDEFINED,
                    .str=str+startStrI+1,
                    .strLen=i-startStrI-1
                };
                listPushBack(tokenList,token);
            }

            startStrI=i;
            LexToken token={
                .e=LEX_TOKEN_PONCTUATION,
                .ponctuation.c=ponctuationTokens[foundIdx]
            };
            listPushBack(tokenList,token);
            continue;
        }

    }


    *pTokens=tokenList;
}

void compile(char *str,size_t strSize)
{
    listType(LexToken) tokens=NULL;
    
    #ifdef NDEBUG
    // lex(&tokens,str,preprocess(str,strSize));
    #else
    size_t preprocessedCodeSize=preprocess(str,strSize);
    printf("preprocessedCode=\"%s\"=preprocessedCode\n",str);

    lex(&tokens,str,preprocessedCodeSize);

    printf("tokens={");
    for(size_t i=0;i<listLength(tokens);++i)
    {
        printf("\n\t{\n\t\t.e=%s",string_LexTokenEnum(tokens[i].e));
        switch(tokens[i].e)
        {
            case LEX_TOKEN_PONCTUATION:
                printf(",\n\t\tchar=\'%c\'",tokens[i].ponctuation.c);
            break;

            case LEX_TOKEN_UNDEFINED:
                printf(",\n\t\t.str=\"%.*s\"",tokens[i].strLen,tokens[i].str);
            break;

        }

        printf("\n\t},");
    }
    printf("\b \n}\n");
    #endif
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

    FILE *inFile=fopen("tests/helloWorld.ln","rb");

    fseek(inFile,0,SEEK_END);
    size_t fileSize=_ftelli64(inFile);
    rewind(inFile);

    char *buf=malloc(fileSize);
    fread(buf,fileSize,1,inFile);
    compile(buf,fileSize);


}

