#include <stdio.h>
#include <string.h>
#include "list.h"
#include "arena.h"
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
    X(LEX_TOKEN_AS)\
    X(LEX_TOKEN_STRUCT)\


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
        char ponctuation; 

        struct
        {
            char *str;
            size_t strLen;
        };
    };
} LexToken;

#define isTokenPonc(token,ponc) (((token).e==LEX_TOKEN_PONCTUATION)&&((token).ponctuation==ponc)) 


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
    X("as")\
    X("struct")\

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


#define AST_NODE_ENUM\
    X(AST_NODE_STATEMENT_LIST)\
    X(AST_NODE_ASSIGNEMENT)\
    X(AST_NODE_EXPRESSION)\
    X(AST_NODE_DEC_VAR)\
    X(AST_NODE_RETURN)\
    X(AST_NODE_VALUE_LIST)\
    X(AST_NODE_DEF_FUNC)\
    X(AST_NODE_CALLING_FUNC)\
    X(AST_NODE_CONSTANT)\
    X(AST_NODE_VAR)\
    X(AST_NODE_MODIFIER)\
    X(AST_NODE_TYPE)\
    X(AST_NODE_PRECISION)\


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

#define AST_NODE_OPERATION_ENUM\
    X(AST_NODE_OPERATION_ADD)\
    X(AST_NODE_OPERATION_SUB)\
    X(AST_NODE_OPERATION_NEG)\
    X(AST_NODE_OPERATION_MUL)\
    X(AST_NODE_OPERATION_DIV)\
    X(AST_NODE_OPERATION_MOD)\
    X(AST_NODE_OPERATION_BOOLEAN_EQUAL)\
    X(AST_NODE_OPERATION_BOOLEAN_NOT_EQUAL)\
    X(AST_NODE_OPERATION_BOOLEAN_LESS_THAN)\
    X(AST_NODE_OPERATION_BOOLEAN_GREATER_THAN)\
    X(AST_NODE_OPERATION_BOOLEAN_EQUAL_OR_GREATER_THAN)\
    X(AST_NODE_OPERATION_BOOLEAN_EQUAL_OR_LESS_THAN)\
    X(AST_NODE_OPERATION_BOOLEAN_AND)\
    X(AST_NODE_OPERATION_BOOLEAN_NOT)\
    X(AST_NODE_OPERATION_BOOLEAN_OR)\
    X(AST_NODE_OPERATION_BOOLEAN_XOR)\

typedef enum AST_NodeOperationEnum
{
    #define X(x) x,
    AST_NODE_OPERATION_ENUM
    #undef X
} AST_NodeOperationEnum;

const char *string_AST_NodeOperationEnum(AST_NodeOperationEnum e)
{
    switch(e)
    {
        #define X(x) case x: return #x;
        AST_NODE_OPERATION_ENUM
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
            AST_Node *node;
            AST_Node *next;
        } statementListNode;

        struct
        {
            AST_Node *leftExpr;
            AST_Node *rightExpr;
        } assignementNode;

        struct
        {
            AST_Node *left;
            AST_Node *right;
            AST_NodeOperationEnum op;
        } expressionNode;

        struct
        {
            const LexToken *nameToken;
            AST_Node *typeNode;
        } decVarNode;

        struct
        {
            AST_Node *expr;
        } returnNode;

        struct
        {
            AST_Node *value;
            AST_Node *next;
        } valueListNode;
    

        // can be dec if code is NULL
        struct
        {
            const LexToken *funcToken;
            AST_Node *typeNode;
            AST_Node *argList;

            AST_Node *code;
        } defFuncNode;

        struct
        {
            AST_Node *func;

            AST_Node *args;
        } callingFuncNode;

        struct
        {
            const LexToken *token;
        } constantNode;

        struct
        {
            const LexToken *token;
        } varNode;

        struct
        {
            const LexToken *modifierToken;

            AST_Node *typeNode;
        } modifierNode;

        struct
        {
            const LexToken *token;
        } typeNode;

        struct
        {
            AST_Node *typeNode;

            AST_Node *expr;
        } precisionNode;
    };
};


#define sizeOfNode(nodeName) (offsetof(AST_Node,statementListNode)+sizeof(((AST_Node){0}). nodeName))

void printLexToken(FILE *file,const LexToken *token)
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

size_t enclosureCheck(char ponctuation)
{
    switch(ponctuation)
    {
        case '{':
        case '[':
        case '(':
            return 1;

        case '}':
        case ']':
        case ')':
            return SIZE_MAX;
    }

    return 0;
}

#define parseFuncArgs const LexToken *tokens,const size_t tokenCount,arenaType(AST_Node) arena

AST_Node *parseExpr(parseFuncArgs);
AST_Node *parseType(parseFuncArgs);
AST_Node *parseList(parseFuncArgs);
AST_Node *parseStatementList(parseFuncArgs);
AST_Node *parseStatement(parseFuncArgs);
AST_Node *parseFile(parseFuncArgs);
// TODO: improve func name
AST_Node *parseFileLine(parseFuncArgs);
AST_Node *parseFunc(parseFuncArgs);



AST_Node *parseType(parseFuncArgs)
{
    if(tokenCount==1)
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(typeNode));
        result->e=AST_NODE_TYPE;

        result->typeNode.token=tokens;

        return result;
    }
    if(tokenCount<3)
    {
        return NULL;
    }

    if(isTokenPonc(tokens[tokenCount-1],']'))
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(precisionNode));
        result->e=AST_NODE_PRECISION;

        size_t startPrecisionCheck=0;
        {
            size_t enclosureCount=0;
            for(size_t i=0;i<tokenCount;++i)
            {
                if(tokens[i].e==LEX_TOKEN_PONCTUATION)
                {
                    char ponc=tokens[i].ponctuation;
                    enclosureCount+=enclosureCheck(ponc);
                    if(
                        ponc=='['&&
                        enclosureCount==1
                    )
                    {
                        startPrecisionCheck=i;
                        break;
                    }
                }
            }
        }

        result->precisionNode.typeNode=parseType(tokens,startPrecisionCheck,arena);

        result->precisionNode.expr=parseExpr(tokens+startPrecisionCheck+1,tokenCount-startPrecisionCheck-2,arena);

        return result;
    }

    AST_Node *result=arenaAlloc(arena,sizeOfNode(modifierNode));
    result->e=AST_NODE_MODIFIER;
    result->modifierNode.modifierToken=tokens;
    result->modifierNode.typeNode=parseType(tokens+2,tokenCount-3,arena);

    return result;
}

AST_Node *parseExpr(parseFuncArgs)
{
    if(tokenCount==0)
    {
        return NULL;
    }

    if(tokenCount==1)
    {
        if(tokens[0].e==LEX_TOKEN_ID)
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(varNode));
            result->e=AST_NODE_VAR;

            result->varNode.token=tokens;

            return result;
        }
        else if(tokens[0].e==LEX_TOKEN_CONSTANT)
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(constantNode));
            result->e=AST_NODE_CONSTANT;

            result->constantNode.token=tokens;

            return result;
        }

        return NULL;
    }

    {
        size_t enclosureCount=0;
        for(size_t i=tokenCount;i!=0;--i)
        {
            if(tokens[i-1].e==LEX_TOKEN_PONCTUATION)
            {
                const char ponc=tokens[i-1].ponctuation;
                enclosureCount+=enclosureCheck(ponc);
                if(ponc=='='&&enclosureCount==0)
                {
                    AST_Node *result=arenaAlloc(arena,sizeOfNode(assignementNode));
                    result->e=AST_NODE_ASSIGNEMENT;

                    result->assignementNode.leftExpr=parseExpr(tokens,i-1,arena);
                    result->assignementNode.rightExpr=parseExpr(tokens+i,tokenCount-i,arena);
                    return result;
                }
            }
        }
    }

    if(tokens[0].e==LEX_TOKEN_ID&&isTokenPonc(tokens[1],':'))
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(decVarNode));
        result->e=AST_NODE_DEC_VAR;

        result->decVarNode.nameToken=tokens;
        result->decVarNode.typeNode=parseType(tokens+2,tokenCount-2,arena);

        return result;
    }

    #define checkOperation(ponctu,oper)\
    {\
        size_t enclosureCount=0;\
        for(size_t i=tokenCount;i!=0;--i)\
        {\
            if(tokens[i-1].e==LEX_TOKEN_PONCTUATION)\
            {\
                const char ponc=tokens[i-1].ponctuation;\
                enclosureCount+=enclosureCheck(ponc);\
                if(ponc==ponctu&&enclosureCount==0)\
                {\
                    AST_Node *result=arenaAlloc(arena,sizeOfNode(expressionNode));\
                    result->e=AST_NODE_EXPRESSION;\
\
                    result->expressionNode.op=AST_NODE_OPERATION_##oper;\
                    result->expressionNode.left=parseExpr(tokens,i-1,arena);\
                    result->expressionNode.right=parseExpr(tokens+i,tokenCount-i,arena);\
                    return result;\
                }\
            }\
        }\
    }

    checkOperation('+',ADD)
    // have to.to handle neg
    {
        size_t enclosureCount=0;
        for(size_t i=tokenCount;i!=0;--i)
        {
            if(tokens[i-1].e==LEX_TOKEN_PONCTUATION)
            {
                const char ponc=tokens[i-1].ponctuation;
                enclosureCount+=enclosureCheck(ponc);
                if(ponc=='-'&&enclosureCount==0)
                {
                    AST_Node *result=arenaAlloc(arena,sizeOfNode(expressionNode));
                    result->e=AST_NODE_EXPRESSION;
                    if(i==1)
                    {
                        result->expressionNode.op=AST_NODE_OPERATION_NEG;
                        result->expressionNode.left=NULL;
                        result->expressionNode.right=parseExpr(tokens+i,tokenCount-1,arena);

                        return result;
                    }
                    result->expressionNode.op=AST_NODE_OPERATION_SUB;
                    result->expressionNode.left=parseExpr(tokens,i-1,arena);
                    result->expressionNode.right=parseExpr(tokens+i,tokenCount-i,arena);
                    return result;
                }
            }
        }
    }
    // checkOperation('-',SUB)
    checkOperation('*',MUL)
    checkOperation('/',DIV)
    checkOperation('%',MOD)

    #undef checkOperation


    if(tokens[0].e==LEX_TOKEN_ID)
    {
        if(isTokenPonc(tokens[1],'('))
        {
            //TODO check for unended enclosures

            if(isTokenPonc(tokens[tokenCount-1],')'))
            {
                // reminder: function needs to be treated as pointers to the code though the assembly should keep them as labels
                AST_Node *result=arenaAlloc(arena,sizeOfNode(callingFuncNode));
            
                result->e=AST_NODE_CALLING_FUNC;

                result->callingFuncNode.func=parseExpr(tokens,1,arena);

                result->callingFuncNode.args=parseList(tokens+2,tokenCount-2,arena);

                return result;
            }
        }
    }

    
    return NULL;


}
AST_Node *parseStatement(parseFuncArgs)
{
    if(tokens[0].e==LEX_TOKEN_RETURN)
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(returnNode));
        result->e=AST_NODE_RETURN;

        if(isTokenPonc(tokens[tokenCount-1],';'))
        {
            result->returnNode.expr=parseExpr(tokens+1,tokenCount-2,arena);
        }
        else
        {
            result->returnNode.expr=parseExpr(tokens+1,tokenCount-1,arena);
        }
        return result;
    }

    return parseExpr(tokens,tokenCount,arena);
}

AST_Node *parseStatementList(parseFuncArgs)
{
    size_t enclosureCount=0;
    for(size_t i=0;i<tokenCount;++i)
    {
        if(tokens[i].e==LEX_TOKEN_PONCTUATION)
        {
            char ponc=tokens[i].ponctuation;
            enclosureCount+=enclosureCheck(ponc);
            if(((ponc==';')||(ponc=='}'))&&enclosureCount==0)
            {
                if(i==tokenCount-1)
                {
                    continue;
                }

                AST_Node *result=arenaAlloc(arena,sizeOfNode(statementListNode));
                result->e=AST_NODE_STATEMENT_LIST;
                
                result->statementListNode.node=parseStatement(tokens,i,arena);

                result->statementListNode.next=parseStatementList(tokens+i+1,tokenCount-i-1,arena);
                
                return result;
            }
        }
    }

    return parseStatement(tokens,tokenCount,arena);
}

AST_Node *parseFileLine(parseFuncArgs)
{
    if(tokens[0].e==LEX_TOKEN_ID&&isTokenPonc(tokens[1],'('))
    {
        return parseFunc(tokens,tokenCount,arena);
    }

    return parseExpr(tokens,tokenCount,arena);
}

AST_Node *parseFile(parseFuncArgs)
{
    size_t enclosureCount=0;
    for(size_t i=0;i<tokenCount;++i)
    {
        if(tokens[i].e==LEX_TOKEN_PONCTUATION)
        {
            char ponc=tokens[i].ponctuation;
            enclosureCount+=enclosureCheck(ponc);
            if(((ponc==';')||(ponc=='}'))&&enclosureCount==0)
            {
                if(i==tokenCount-1)
                {
                    continue;
                }

                AST_Node *result=arenaAlloc(arena,sizeOfNode(statementListNode));
                result->e=AST_NODE_STATEMENT_LIST;
                
                if(ponc=='}')
                {
                    result->statementListNode.node=parseFileLine(tokens,i+1,arena);
                }
                //then ';'
                else
                {
                    result->statementListNode.node=parseFileLine(tokens,i,arena);
                }
                
                result->statementListNode.next=parseFile(tokens+i+1,tokenCount-i-1,arena);

                return result;
            }
        }
    }

    if(isTokenPonc(tokens[tokenCount-1],';'))
    {
        return parseFileLine(tokens,tokenCount-1,arena);
    }
    else
    {
        return parseFileLine(tokens,tokenCount,arena);
    }
}


AST_Node *parseList(parseFuncArgs)
{
    if(tokenCount==0)
    {
        return NULL;
    }
    
    

    size_t enclosureCount=0;

    for(size_t i=0;i<tokenCount;++i)
    {
        if(tokens[i].e==LEX_TOKEN_PONCTUATION)
        {
            char ponc=tokens[i].ponctuation;
            enclosureCount+=enclosureCheck(ponc);

            if(enclosureCount==0&&ponc==',')
            {
                if(i==tokenCount-1)
                {
                    continue;
                }

                AST_Node *result=arenaAlloc(arena,sizeOfNode(valueListNode));
                result->e=AST_NODE_VALUE_LIST;
                result->valueListNode.value=parseExpr(tokens,i,arena);

                result->valueListNode.next=parseList(tokens+i+1,tokenCount-i-1,arena);

                return result;
            }
        }
    }


    return parseExpr(tokens,tokenCount,arena);
}

AST_Node *parseFunc(parseFuncArgs)
{
    AST_Node *result=arenaAlloc(arena,sizeOfNode(defFuncNode));
    result->e=AST_NODE_DEF_FUNC;

    result->defFuncNode.funcToken=tokens;

    // points at the ')'
    size_t endArgList=0;
    {
        size_t enclosureCount=0;;
        for(size_t i=1;i<tokenCount;++i)
        {
            if(tokens[i].e==LEX_TOKEN_PONCTUATION)
            {
                char ponc=tokens[i].ponctuation;
                enclosureCount+=enclosureCheck(ponc);

                if(enclosureCount==0&&ponc==')')
                {
                    endArgList=i;
                    break;
                }
            }
        }
    }


    if(endArgList==0)
    {
        return NULL;
    }

    result->defFuncNode.argList=parseList(tokens+2,endArgList-2,arena);

    size_t startCodeIdx=0;
    // '='
    bool onlyExpression=false;
    {
        size_t enclosureCount=0;
        for(size_t i=endArgList+1;i<tokenCount;++i)
        {
            if(tokens[i].e==LEX_TOKEN_PONCTUATION)
            {
                char ponc=tokens[i].ponctuation;
                enclosureCount+=enclosureCheck(ponc);

                if(enclosureCount==1&&ponc=='{')
                {
                    startCodeIdx=i;
                    onlyExpression=false;
                    break;
                }

                if(enclosureCount==0&&ponc=='=')
                {
                    startCodeIdx=i;
                    onlyExpression=true;
                    break;
                }
            }
        }
    }
    if(startCodeIdx==0)
    {
        result->defFuncNode.typeNode=parseType(tokens+endArgList+2,tokenCount-endArgList-2,arena);
        result->defFuncNode.code=NULL;

        return result;
    }

    result->defFuncNode.typeNode=parseType(tokens+endArgList+2,startCodeIdx-endArgList-2,arena);

    if(onlyExpression)
    {
        result->defFuncNode.code=arenaAlloc(arena,sizeOfNode(returnNode));
        result->defFuncNode.code->e=AST_NODE_RETURN;

        result->defFuncNode.code->returnNode.expr=parseExpr(tokens+startCodeIdx+1,tokenCount-startCodeIdx-1,arena);
    }
    else
    {
        result->defFuncNode.code=parseStatementList(tokens+startCodeIdx+1,tokenCount-startCodeIdx-2,arena);
    }
    return result;
}


bool parse(listType(LexToken) tokenList,arenaType(AST_Node) arena,AST_Node **start)
{
    *start=parseFile(tokenList,listLength(tokenList),arena);

    return *start==NULL;
    
}


#define printTreeExpr for(size_t depthIdx=0;depthIdx<depth;++depthIdx){printf("     ");}

void printTree(AST_Node *node)
{
    static int depth=0;

    if(depth>100)
    {
        fprintf(stderr,"something's fishy");
    }
    if(!node)
    {
        printTreeExpr
        printf("returned NULL :(\n");
        return;
    }
    switch(node->e)
    {
        case AST_NODE_STATEMENT_LIST:
            printTreeExpr
            printf("statementList:\n");

            depth++;
            if(node->statementListNode.node->e==AST_NODE_STATEMENT_LIST)
            {
                depth--;
            }
            printTree(node->statementListNode.node);
            if(node->statementListNode.next!=NULL)
            {
                printTree(node->statementListNode.next);
            }

            depth--;

            if(node->statementListNode.node->e==AST_NODE_STATEMENT_LIST)
            {
                depth++;
            }
        break;

        case AST_NODE_ASSIGNEMENT:
            printTreeExpr
            printf("assignementNode:\n");
            depth++;

            printTreeExpr
            printf("leftExpr:\n");

            depth++;
            printTree(node->assignementNode.leftExpr);
            depth--;

            printTreeExpr
            printf("rightExpr:\n");

            depth++;
            printTree(node->assignementNode.rightExpr);
            depth--;

            depth--;
        break;

        case AST_NODE_EXPRESSION:
            printTreeExpr
            printf("expression:\n");

            depth++;

            printTreeExpr
            printf("op:%s\n",string_AST_NodeOperationEnum(node->expressionNode.op));
            
            if(node->expressionNode.left!=NULL)
            {
                printTreeExpr
                printf("left:\n");

                depth++;

                printTree(node->expressionNode.left);

                depth--;
            }
            printTreeExpr
            printf("right:\n");

            depth++;
            printTree(node->expressionNode.right);

            depth--;

            depth--;
        break;

        case AST_NODE_DEC_VAR:
            printTreeExpr    
            printf("decVar:\n");

            depth++;
            printTreeExpr
            printf("nameToken=");
            printLexToken(stdout,node->decVarNode.nameToken);
            printf("\n");

            printTreeExpr
            printf("typeNode:\n");
            depth++;
            printTree(node->decVarNode.typeNode);
            depth--;

            depth--;
        break;

        case AST_NODE_RETURN:
            printTreeExpr
            printf("return:\n");
            
            depth++;
            printTree(node->returnNode.expr);
            depth--;
        break;

        case AST_NODE_VALUE_LIST:
            printTreeExpr
            printf("valueList:\n");

            depth++;
            if(node->valueListNode.value->e==AST_NODE_VALUE_LIST)
            {
                depth--;
            }
            printTree(node->valueListNode.value);
            if(node->valueListNode.next!=NULL)
            {
                printTree(node->valueListNode.next);
            }

            depth--;

            if(node->valueListNode.value->e==AST_NODE_VALUE_LIST)
            {
                depth++;
            }
        break;

        case AST_NODE_DEF_FUNC:
            printTreeExpr
            printf("defFunc:\n");


            depth++;

            printTreeExpr
            printf("func:\"");
            printLexToken(stdout,node->defFuncNode.funcToken);
            printf("\"\n");


            printTreeExpr
            printf("argList:\n");
            depth++;
                printTree(node->defFuncNode.argList);
            depth--;

            printTreeExpr
            printf("type:\n");
            depth++;
            printTree(node->defFuncNode.typeNode);
            depth--;

            
            printTreeExpr
            printf("code:\n");
            depth++;
            printTree(node->defFuncNode.code);
            depth--;

            depth--;
        break;

        case AST_NODE_CALLING_FUNC:
            printTreeExpr
            printf("callingFunc:\n");

            depth++;

            printTreeExpr
            printf("func:\n");
            depth++;
            printTree(node->callingFuncNode.func);
            depth--;

            printTreeExpr
            printf("args:\n");
            depth++;
            printTree(node->callingFuncNode.args);
            depth--;
            
            depth--;
        break;

        case AST_NODE_CONSTANT:
            printTreeExpr
            printf("constant:\n");

            depth++;

            printTreeExpr
            printf("token:\"");

            printLexToken(stdout,node->constantNode.token);
            printf("\"\n");

            depth--;
        break;

        case AST_NODE_VAR:
            printTreeExpr
            printf("varNode:\n");
            
            depth++;

            printTreeExpr
            printf("token:\"");

            printLexToken(stdout,node->varNode.token);
            printf("\"\n");

            depth--;
        break;

        case AST_NODE_MODIFIER:
            printTreeExpr
            printf("modifier:\n");
            
            depth++;

            printTreeExpr
            printf("modifierToken:\"");

            printLexToken(stdout,node->modifierNode.modifierToken);
            printf("\"\n");

            printTreeExpr
            printf("typeNode:\n");
            depth++;
            printTree(node->modifierNode.typeNode);
            depth--;


            depth--;
        break;

        case AST_NODE_TYPE:
            printTreeExpr
            printf("type:\n");
            
            depth++;

            printTreeExpr
            printf("token:\"");

            printLexToken(stdout,node->typeNode.token);
            printf("\"\n");


            depth--;
        break;

        case AST_NODE_PRECISION:
            printTreeExpr
            printf("precision:\n");

            depth++;

            printTreeExpr
            printf("typeNode\n");
            depth++;
            printTree(node->precisionNode.typeNode);
            depth--;
            
            printTreeExpr
            printf("expr:\n");
            depth++;
            printTree(node->precisionNode.expr);
            depth--;

            depth--;
        break;


        default:
            printTreeExpr
            printf("e(str):%s\n",string_AST_NodeEnum(node->e));
        break;

    }
}


void cleanUpTree()
{
    
}


bool isCodeValid(AST_Node *head)
{
    // TODO: implement it
    return true;
}


void generateAssembly(FILE *outFile,AST_Node *head)
{
    switch(head->e)
    {
        case AST_NODE_STATEMENT_LIST:
            generateAssembly(outFile,head->statementListNode.node);

            generateAssembly(outFile,head->statementListNode.next);

        break;

        case AST_NODE_DEF_FUNC:

            printLexToken(outFile,head->defFuncNode.funcToken);
            fprintf(outFile,":\n");

            generateAssembly(outFile,head->defFuncNode.code);

        break;

        case AST_NODE_RETURN:
            if(head->returnNode.expr->e!=AST_NODE_CONSTANT)
            {
                fprintf(stderr,"isn't a constant");
                exit(EXIT_FAILURE);
            }

            fprintf(outFile,"mov eax,");
            printLexToken(outFile,head->returnNode.expr->constantNode.token);
            fprintf(outFile,"\n");
            

            fprintf(outFile,"ret\n");
        break;

    }
}


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
                printf(",\n\t\tchar=\'%c\'",tokens[i].ponctuation);
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

    arenaType(void) parseTreeArena;
    arenaCreate(parseTreeArena,1ull<<15);

    AST_Node *treeRoot=NULL;
    parse(tokens,parseTreeArena,&treeRoot);

    printTree(treeRoot);

    if(!isCodeValid(treeRoot))
    {
        exit(EXIT_FAILURE);
    }

    // fputs("section .text\nglobal WinMain\nWinMain:\ncall main\nret\n",outFile);

    // generateAssembly(outFile,treeRoot);


    #endif
}





int main(int argc,char *argv[])
{
    FILE *outFile=fopen("compiler/out/cCompiler.asm","wb");


    FILE *inFile=fopen("tests/compilePrecisionCheck.ln","rb");

    fseek(inFile,0,SEEK_END);
    size_t fileSize=_ftelli64(inFile);
    rewind(inFile);

    char *buf=malloc(fileSize);
    fread(buf,fileSize,1,inFile);
    compile(buf,fileSize,outFile);


}

