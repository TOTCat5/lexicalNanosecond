#include "parse.h"


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
    if(tokenCount==0)
    {
        return NULL;
    }

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

    if(tokens[0].e==LEX_TOKEN_IF)
    {
        if(!isTokenPonc(tokens[1],'('))
        {
            return NULL;
        }

        size_t endIfExprIdx=0;
        {
            size_t enclosureCount=0;
            for(size_t i=2;i<tokenCount;++i)
            {
                if(tokens[i].e==LEX_TOKEN_PONCTUATION)
                {
                    char ponc=tokens[i].ponctuation;
                    enclosureCount+=enclosureCheck(ponc);

                    if(enclosureCount==0&&ponc==')')
                    {
                        endIfExprIdx=i;
                        break;
                    }
                }
            }
        }
        // if(endIfExprIdx!=0)
        // {
        //     AST_Node *result=arenaAlloc(arena,sizeOfNode(ifElseNode));
        //     result->e=

        // }
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