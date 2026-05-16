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
    X(LEX_TOKEN_AS)


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
    '&',
    '^',
    '|',
    '!'
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


#define KEYWORDS_LIST\
    X("import")\
    X("for")\
    X("from")\
    X("to")\
    X("until")\
    X("while")\
    X("if")\
    X("return")\
    X("as")

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
                .ponctuation.c=ponctuationTokens[foundIdx]
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
    size_t tokenListSize=listLength(tokenList);
    for(size_t i=0;i<tokenListSize;++i)
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

            bool isDecimalNumber=isTokenDecimalInteger(tokenList+i);
            if(isDecimalNumber)
            {
                tokenList[i].e=LEX_TOKEN_CONSTANT;
                if(i<tokenListSize-2)
                {
                    bool isFloatNumber=tokenList[i+1].e==LEX_TOKEN_PONCTUATION&&
                                        tokenList[i+1].ponctuation.c=='.'&&
                                        isTokenDecimalInteger(tokenList+i+2);
                    if(isFloatNumber)
                    {
                        listRemoveAtIndex(tokenList,i+1);
                        tokenList[i].strLen+=1+tokenList[i+1].strLen;
                        listRemoveAtIndex(tokenList,i+1);
                        tokenListSize-=2;
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

        if(tokenList[i].e==LEX_TOKEN_PONCTUATION&&tokenList[i].ponctuation.c==' ')
        {
            listRemoveAtIndex(tokenList,i);
            i-=1;
            tokenListSize--;
        }
    }


    *pTokens=tokenList;
}


#define AST_NODE_ENUM\
    X(AST_NODE_CONSTANT)\
    X(AST_NODE_BINARY_OPERATION)\
    X(AST_NODE_IF)\
    X(AST_NODE_VARIABLE_ASSIGNMENT)\
    X(AST_NODE_STATEMENT_LIST_NODE)\
    X(AST_NODE_RETURN)\
    X(AST_NODE_CALL_FUNC)\
    X(AST_NODE_DEF_FUNC)\



typedef enum AST_NodeEnum
{
    #define X(x) x,
    AST_NODE_ENUM
    #undef X
} AST_NodeEnum;

const char *string_AST_NodeEnum(AST_NodeEnum e)
{
    switch(e)
    {
        #define X(x) case x: return #x;
        AST_NODE_ENUM
        #undef X
    }
    return __func__;
}

#define AST_NODE_BINARY_CONDITION_ENUM\
    X(AST_NODE_BINARY_CONDITION_EQUAL)\
    X(AST_NODE_BINARY_CONDITION_LESS_THAN)\
    X(AST_NODE_BINARY_CONDITION_GREATER_THAN)\
    X(AST_NODE_BINARY_CONDITION_EQUAL_OR_GREATER_THAN)\
    X(AST_NODE_BINARY_CONDITION_EQUAL_OR_LESS_THAN)\

typedef enum AST_NodeBinaryConditionEnum
{
    #define X(x) x,
    AST_NODE_BINARY_CONDITION_ENUM
    #undef X
} AST_NodeBinaryConditionEnum;

const char *string_AST_NodeBinaryConditionEnum(AST_NodeBinaryConditionEnum e)
{
    switch(e)
    {
        #define X(x) case x: return #x;
        AST_NODE_BINARY_CONDITION_ENUM
        #undef X
    }
    return __func__;
}
    


typedef struct AST_Node AST_Node;
struct AST_Node
{
    AST_NodeEnum e;
    union
    {
        struct
        {
            LexToken *token;
        } constantNode;

        struct
        {
            AST_Node *leftExpr;
            AST_NodeBinaryConditionEnum e;
            AST_Node *rightExpr;
        } binaryConditionNode;

        struct
        {
            AST_Node *condition;
            AST_Node *ifNode;
            AST_Node *elseNode;
        } ifNode;

        struct
        {
            AST_Node *var;
            AST_Node *expr;
        } varAssignmentNode;
        
        struct
        {
            AST_Node *statement;
            AST_Node *next;
        } statementListNodeNode;

        struct
        {

        } callFuncNode;

        struct
        {
            AST_Node *executedNode;
            AST_Node *callArgListNode;
        } defFuncNode;
    };
};




void compile(char *str,size_t strSize,FILE *outFile)
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

            case LEX_TOKEN_CONSTANT:
                printf(",\n\t\t.str=\"%.*s\"",tokens[i].strLen,tokens[i].str);
            break;

            case LEX_TOKEN_ID:
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


    FILE *inFile=fopen("tests/VGA_ScreenWriting.ln","rb");

    fseek(inFile,0,SEEK_END);
    size_t fileSize=_ftelli64(inFile);
    rewind(inFile);

    char *buf=malloc(fileSize);
    fread(buf,fileSize,1,inFile);
    compile(buf,fileSize,outFile);


}

