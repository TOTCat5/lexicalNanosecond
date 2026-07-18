#include "lex.h"




bool toggleString(char *str,size_t i,size_t strSize)
{
    if(str[i]=='\"')
    {
        bool hasBackslash=false;
        size_t backslashI=i;
        while(backslashI>1)
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
            out[outI++]=' ';
            ++outSize;
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
        char ponctuation; 

        struct
        {
            char *str;
            size_t strLen;
        };
    };
} LexToken;



uint16_t ponctuationTokens[]={
    ' ',
    ',',
    '.',
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
    '%',
    '&',
    '^',
    '|',
    '!'
};



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

bool isTokenDecimalInteger(LexToken *token)
{
    if(token->e!=LEX_TOKEN_PONCTUATION)
    {
        for(size_t i=0;i<token->strLen;++i)
        {
            char t=token->str[i];
            if(!(t>='0'&&t<='9'))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool isTokenString(LexToken *token)
{
    if(token->strLen<2)
    {
        return false;
    }

    if(
        (token->str[0]=='\"')&&
        (token->str[token->strLen-1]=='\"')
    )
    {
        return true;
    }
    else
    {
        return false;
    }

    return true;
}

bool isTokenHexInteger(LexToken *token)
{
    if(token->e!=LEX_TOKEN_PONCTUATION&&token->strLen>2)
    {
        if(token->str[0]=='0'&&token->str[1]=='x')
        {
            for(size_t i=token->strLen-1;i>2;--i)
            {
                char t=token->str[i];
                if(!(
                    (t>='0'&&t<='9')||
                    (t>='a'&&t<='f')
                ))
                {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}


// Assume str is null-terminated
void lex(listType(LexToken) *pTokens,char *str,size_t strSize)
{
    LexToken *tokenList=NULL;
    listCreate(tokenList,2);

    size_t startStrI=-1;

    bool isInString=false;
    bool wasInString=isInString;
    for(size_t i=0;i<strSize;++i)
    {
        isInString^=toggleString(str,i,strSize);
        if(isInString)
        {
            continue;
        }
        if((!isInString)&&wasInString)
        {
            LexToken token={
                .e=LEX_TOKEN_CONSTANT,
                .str=str+startStrI,
                .strLen=i-startStrI+1
            };
            startStrI=i;
            listPushBack(tokenList,token);
        }
        wasInString=isInString;

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
                .ponctuation=ponctuationTokens[foundIdx]
            };
            listPushBack(tokenList,token);
            continue;
        }

    }

    #define X(x) x,
    char *keywords[]={
        KEYWORDS_LIST
    };
    #undef X
    #define X(x) sizeof(x)-1,
    size_t lengths[]={
        KEYWORDS_LIST
    };
    #undef X
    for(size_t i=0;i<listLength(tokenList);++i)
    {
        if(tokenList[i].e==LEX_TOKEN_UNDEFINED)
        {
            size_t keywordIdx=SIZE_MAX;
            for(size_t j=0;j<sizeof(keywords)/sizeof(*keywords);++j)
            {
                if(lengths[j]==tokenList[i].strLen)
                {
                    if(strncmp(keywords[j],tokenList[i].str,lengths[j])==0)
                    {
                        keywordIdx=j;
                        break;
                    }
                }
            }
            if(keywordIdx!=SIZE_MAX)
            {
                tokenList[i].e=LEX_TOKEN_IMPORT+keywordIdx;
                continue;
            }

            bool isAString=isTokenString(tokenList+i);
            if(isAString)
            {
                tokenList[i].e=LEX_TOKEN_CONSTANT;
                continue;
            }

            bool isDecimalNumber=isTokenDecimalInteger(tokenList+i);
            if(isDecimalNumber)
            {
                tokenList[i].e=LEX_TOKEN_CONSTANT;
                if(i<listLength(tokenList)-2)
                {
                    bool isFloatNumber=tokenList[i+1].e==LEX_TOKEN_PONCTUATION&&
                                        tokenList[i+1].ponctuation=='.'&&
                                        isTokenDecimalInteger(tokenList+i+2);
                    if(isFloatNumber)
                    {
                        listRemoveAtIndex(tokenList,i+1);
                        tokenList[i].strLen+=1+tokenList[i+1].strLen;
                        listRemoveAtIndex(tokenList,i+1);
                        i-=2;
                    }
                }
                continue;
            }

            bool isHexNumber=isTokenHexInteger(tokenList+i);
            if(isHexNumber)
            {
                tokenList[i].e=LEX_TOKEN_CONSTANT;
                continue;
            }

            tokenList[i].e=LEX_TOKEN_ID;

        }

        if(tokenList[i].e==LEX_TOKEN_PONCTUATION&&tokenList[i].ponctuation==' ')
        {
            listRemoveAtIndex(tokenList,i);
            i-=1;
        }
    }


    *pTokens=tokenList;
}