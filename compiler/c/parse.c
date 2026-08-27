#include "parse.h"



// Basically parseExpr with a Parentheses Check
AST_Node *parseExprWithPotentialEnclosure(parseFuncArgs);
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
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    if(
        (nodeListLength==1)&&
        !tokens[0].isNewEnclosure
    )
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(typeNode));
        result->e=AST_NODE_TYPE;

        result->typeNode.token=tokens[0].token;

        return result;
    }
    
    if(nodeListLength>3)
    {
        return NULL;
    }
    

    if(hasEnclosureOfPonc(tokens[nodeListLength-1],'['))
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(precisionNode));
        result->e=AST_NODE_PRECISION;

        result->precisionNode.typeNode=parseType(
            tokens->node.list,
            nodeListLength-1,
            arena
        );
        result->precisionNode.expr=parseExpr(
            tokens+nodeListLength-1,
            nodeListLength,
            arena
        );



        return result;
    }

    AST_Node *result=arenaAlloc(arena,sizeOfNode(modifierTypeNode));
    result->e=AST_NODE_MODIFIER_TYPE;

    result->modifierTypeNode.modifierToken=tokens[0].token;
    result->modifierTypeNode.typeNode=parseType(tokens[1].node.list,0,arena);

    return result;
}

AST_Node *parseExprWithPotentialEnclosure(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    if(
        nodeListLength==1&&
        tokens[0].isNewEnclosure
    )
    {
        return parseExpr(tokens[0].node.list,0,arena);
    }

    return parseExpr(tokens,nodeListLength,arena);
}

AST_Node *parseExpr(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    if(nodeListLength==0)
    {
        return NULL;
    }

    if(nodeListLength==1)
    {
        const LexToken *token=tokens->token;

        if(token->e==LEX_TOKEN_ID)
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(varNode));
            result->e=AST_NODE_VAR;

            result->varNode.token=token;

            return result;
        }
        else if(token->e==LEX_TOKEN_CONSTANT)
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(constantNode));
            result->e=AST_NODE_CONSTANT;

            result->constantNode.token=token;

            return result;
        }

        return NULL;
    }

    {
        for(size_t i=nodeListLength;i!=0;--i)
        {
            if(!tokens[i-1].isNewEnclosure)
            {
                if(tokens[i-1].token->e==LEX_TOKEN_PONCTUATION)
                {
                    const char ponc=getEnclosureToken(tokens+i-1)->ponctuation;
                    if(ponc=='=')
                    {
                        AST_Node *result=arenaAlloc(arena,sizeOfNode(assignementNode));
                        result->e=AST_NODE_ASSIGNEMENT;

                        result->assignementNode.leftExpr=parseExpr(tokens,i-1,arena);
                        result->assignementNode.rightExpr=parseExprWithPotentialEnclosure(tokens+nodeListLength-i,i,arena);
                        return result;
            
                    }
                }
            }
        }

        // size_t enclosureCount=0;
        // for(size_t i=tokenCount;i!=0;--i)
        // {
        //     if(tokens[i-1].e==LEX_TOKEN_PONCTUATION)
        //     {
        //         const char ponc=tokens[i-1].ponctuation;
        //         enclosureCount+=enclosureCheck(ponc);
        //         if(ponc=='='&&enclosureCount==0)
        //         {
        //             AST_Node *result=arenaAlloc(arena,sizeOfNode(assignementNode));
        //             result->e=AST_NODE_ASSIGNEMENT;

        //             result->assignementNode.leftExpr=parseExpr(tokens,i-1,arena);
        //             result->assignementNode.rightExpr=parseExpr(tokens+i,tokenCount-i,arena);
        //             return result;
        //         }
        //     }
        // }
    }

    if(
        getEnclosureToken(tokens)->e==LEX_TOKEN_ID&&
        !tokens[1].isNewEnclosure
    )
    {
        if(isTokenPonc(*tokens[1].token,':'))
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(decVarNode));
            result->e=AST_NODE_DEC_VAR;

            result->decVarNode.nameToken=getEnclosureToken(tokens);
            result->decVarNode.typeNode=parseType(tokens+2,nodeListLength-2,arena);

            return result;
        }
        
    }

    #define checkOperation(ponctu,oper)\
    {\
        for(size_t i=nodeListLength;i!=0;--i)\
        {\
            if(!tokens[i-1].isNewEnclosure)\
            {\
                if(tokens[i-1].token->e==LEX_TOKEN_PONCTUATION)\
                {\
                    const char ponc=tokens[i-1].token->ponctuation;\
                    if(ponc==ponctu)\
                    {\
                        AST_Node *result=arenaAlloc(arena,sizeOfNode(expressionNode));\
                        result->e=AST_NODE_EXPRESSION;\
    \
                        result->expressionNode.op=AST_NODE_OPERATION_##oper;\
                        result->expressionNode.left=parseExprWithPotentialEnclosure(tokens,i-1,arena);\
                        result->expressionNode.right=parseExprWithPotentialEnclosure(tokens+i,nodeListLength-i,arena);\
    \
                        return result;\
                    }\
                }\
            }\
        }\
    }

    checkOperation('+',ADD)
    // have to.to handle neg
    for(size_t i=nodeListLength;i!=0;--i)\
    {
        if(!tokens[i-1].isNewEnclosure)
        {
            if(tokens[i-1].token->e==LEX_TOKEN_PONCTUATION)
            {
                const char ponc=tokens[i-1].token->ponctuation;
                if(ponc=='-')
                {
                    AST_Node *result=arenaAlloc(arena,sizeOfNode(expressionNode));
                    result->e=AST_NODE_EXPRESSION;
                    if(i==1)
                    {
                        result->expressionNode.op=AST_NODE_OPERATION_NEG;
                        result->expressionNode.left=NULL;

                        result->expressionNode.right=parseExprWithPotentialEnclosure(tokens+1,nodeListLength-1,arena);
                        

                        return result;
                    }
                    result->expressionNode.op=AST_NODE_OPERATION_SUB;

                    result->expressionNode.left=parseExprWithPotentialEnclosure(tokens,i-1,arena);
                    
                    
                    if(i==2&&hasEnclosureOfPonc(tokens[i],'('))
                    {
                        result->expressionNode.right=parseExpr(tokens[i].node.list,0,arena);
                    }
                    else
                    {
                        result->expressionNode.right=parseExpr(tokens+i,nodeListLength-i,arena);
                    }
                
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

    
    if(hasEnclosureOfPonc(tokens[nodeListLength-1],'('))
    {
        if(getEnclosureToken(tokens)->e==LEX_TOKEN_ID)
        {
            // reminder: function needs to be treated as pointers to the code though the assembly should keep them as labels
            AST_Node *result=arenaAlloc(arena,sizeOfNode(callingFuncNode));
        
            result->e=AST_NODE_CALLING_FUNC;

            result->callingFuncNode.func=parseExpr(tokens,nodeListLength-1,arena);

            result->callingFuncNode.args=parseList(tokens[nodeListLength-1].node.list,0,arena);

            return result;
        }
    }

    
    return NULL;


}
AST_Node *parseStatement(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    if(nodeListLength==0)
    {
        return NULL;
    }

    if(getEnclosureToken(tokens)->e==LEX_TOKEN_RETURN)
    {
        AST_Node *result=arenaAlloc(arena,sizeOfNode(returnNode));
        result->e=AST_NODE_RETURN;

        if(isTokenPonc(*getEnclosureToken(tokens+nodeListLength-1),';'))
        {
            result->returnNode.expr=parseExprWithPotentialEnclosure(tokens+1,nodeListLength-2,arena);
        }
        else
        {
            result->returnNode.expr=parseExprWithPotentialEnclosure(tokens+1,nodeListLength-1,arena);
        }
        return result;
    }

    // Put back support to if statement
    // if(getEnclosureToken(tokens)->e==LEX_TOKEN_IF)
    // {
    //     if(!hasEnclosureOfPonc(tokens[1],'('))
    //     {
    //         return NULL;
    //     }
    // }

    if(isTokenPonc(*getEnclosureToken(tokens+nodeListLength-1),';'))
    {
        return parseExprWithPotentialEnclosure(tokens,nodeListLength-1,arena);
    }

    return parseExprWithPotentialEnclosure(tokens,nodeListLength,arena);
}

AST_Node *parseStatementList(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    size_t enclosureCount=0;
    for(size_t i=0;i<nodeListLength;++i)
    {
        #define thing\
            if(i==nodeListLength-1)\
            {\
                continue;\
            }\
\
            AST_Node *result=arenaAlloc(arena,sizeOfNode(statementListNode));\
            result->e=AST_NODE_STATEMENT_LIST;\
\
            result->statementListNode.node=parseStatement(tokens,i,arena);\
\
            result->statementListNode.next=parseStatementList(tokens+i+1,nodeListLength-i-1,arena);\
\
            return result;\


        if(tokens[i].isNewEnclosure)
        {
            if(tokens->node.enclosurePonc!='{')
            {
                continue;
            }

            thing
        }
        
        char ponc=getEnclosureToken(tokens+i)->ponctuation;
        if(ponc==';')
        {
            thing
        }

        #undef thing
    }

    return parseStatement(tokens,nodeListLength,arena);
}

// kind of a stretch to use "line" but who gives a shit
AST_Node *parseFileLine(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    if(!tokens[0].isNewEnclosure)
    {
        if(
            getEnclosureToken(tokens+0)->e==LEX_TOKEN_STRUCT&&
            getEnclosureToken(tokens+1)->e==LEX_TOKEN_ID&&
            hasEnclosureOfPonc(tokens[2],'{')
        )
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(structNode));
            result->e=AST_NODE_STRUCT;

            result->structNode.nameToken=getEnclosureToken(tokens+1);
            result->structNode.fieldList=parseStatementList(tokens[2].node.list,0,arena);

            return result;
        }

        if(
            getEnclosureToken(tokens+0)->e==LEX_TOKEN_MODIFIER&&
            hasEnclosureOfPonc(tokens[1],'(')
        )
        {
            AST_Node *result=arenaAlloc(arena,sizeOfNode(modifierNode));
            result->e=AST_NODE_MODIFIER;
            
            result->modifierNode.typeWritten=getEnclosureToken(tokens[1].node.list);

            // assuke only one type for a modifier so bad if i plan to add more types to modifier 
            result->modifierNode.code=parseFileLine(tokens+3,nodeListLength-4,arena);

            return result;
        }



        if(
            getEnclosureToken(tokens+0)->e==LEX_TOKEN_ID&&
            hasEnclosureOfPonc(tokens[1],'(')    
        )
        {
            return parseFunc(tokens,nodeListLength,arena);
        }    
    }

    


    return parseExpr(tokens,nodeListLength,arena);
}

AST_Node *parseFile(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    size_t enclosureCount=0;
    for(size_t i=0;i<nodeListLength;++i)
    {
        #define thing\
            if(i==nodeListLength-1)\
            {\
                continue;\
            }\
\
            AST_Node *result=arenaAlloc(arena,sizeOfNode(statementListNode));\
            result->e=AST_NODE_STATEMENT_LIST;\
\
            result->statementListNode.node=parseFileLine(tokens,i,arena);\
\
            result->statementListNode.next=parseFile(tokens+i+1,nodeListLength-i-1,arena);\
\
            return result;\


        if(tokens[i].isNewEnclosure)
        {
            if(tokens[i].node.enclosurePonc!='{')
            {
                continue;
            }

            if(i==nodeListLength-1)
            {
                continue;
            }

            AST_Node *result=arenaAlloc(arena,sizeOfNode(statementListNode));
            result->e=AST_NODE_STATEMENT_LIST;

            result->statementListNode.node=parseFileLine(tokens,i+1,arena);

            result->statementListNode.next=parseFile(tokens+i+1,nodeListLength-i-1,arena);

            return result;
        }
        
        char ponc=getEnclosureToken(tokens+i)->ponctuation;
        if(ponc==';')
        {
            if(i==nodeListLength-1)
            {
                continue;
            }

            AST_Node *result=arenaAlloc(arena,sizeOfNode(statementListNode));
            result->e=AST_NODE_STATEMENT_LIST;

            result->statementListNode.node=parseFileLine(tokens,i,arena);

            result->statementListNode.next=parseFile(tokens+i+1,nodeListLength-i-1,arena);

            return result;
        }

        #undef thing
    }

    if(
        !tokens[nodeListLength-1].isNewEnclosure&&
        isTokenPonc(*(tokens[nodeListLength-1].token),';')
    )
    {
        return parseFileLine(tokens,nodeListLength-1,arena);    
    }

    return parseFileLine(tokens,nodeListLength,arena);
}


AST_Node *parseList(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    if(nodeListLength==0)
    {
        return NULL;
    }
    
    
    for(size_t i=0;i<nodeListLength;++i)
    {
        if(tokens[i].isNewEnclosure)
        {
            continue;
        }

        if(tokens[i].token->e==LEX_TOKEN_PONCTUATION)
        {
            char ponc=tokens[i].token->ponctuation;
            
            if(ponc==',')
            {
                if(i==nodeListLength-1)
                {
                    continue;
                }

                AST_Node *result=arenaAlloc(arena,sizeOfNode(valueListNode));
                result->e=AST_NODE_VALUE_LIST;
                result->valueListNode.value=parseExprWithPotentialEnclosure(tokens,i,arena);

                result->valueListNode.next=parseList(tokens+i+1,nodeListLength-i-1,arena);

                return result;
            }
        }
    }


    return parseExprWithPotentialEnclosure(tokens,nodeListLength,arena);
}

AST_Node *parseFunc(parseFuncArgs)
{
    size_t nodeListLength=listLength;
    if(!listLength)
    {
        nodeListLength=listLength(tokens);
    }

    AST_Node *result=arenaAlloc(arena,sizeOfNode(defFuncNode));
    result->e=AST_NODE_DEF_FUNC;

    result->defFuncNode.funcToken=tokens[0].token;

    result->defFuncNode.argList=parseList(tokens[1].node.list,0,arena);

    size_t startCodeIdx=0;

    for(size_t i=3;i<nodeListLength;++i)
    {
        if(
            hasEnclosureOfPonc(tokens[i],'{')||
            (
                !tokens[i].isNewEnclosure&&
                isTokenPonc(*(tokens[i].token),'=')
            )
        )
        {
            startCodeIdx=i;
            break;
        }
    }

    if(startCodeIdx==0)
    {
        //                                              -3 for the "name():" + ';' 
        result->defFuncNode.typeNode=parseType(tokens+3,nodeListLength-4,arena);
        result->defFuncNode.code=NULL;

        return result;
    }
    
    result->defFuncNode.typeNode=parseType(tokens+3,startCodeIdx-3,arena);

    if(!hasEnclosureOfPonc(tokens[startCodeIdx],'{'))
    {
        // so '='

        result->defFuncNode.code=arenaAlloc(arena,sizeOfNode(returnNode));
        result->defFuncNode.code->e=AST_NODE_RETURN;

        result->defFuncNode.code->returnNode.expr=parseExprWithPotentialEnclosure(tokens+startCodeIdx+1,nodeListLength-startCodeIdx-1,arena);
    
        return result;
    }


    result->defFuncNode.code=parseStatementList(tokens[startCodeIdx].node.list,0,arena);

    return result;
}


bool parse(EnclosureTreeNode *node,arenaType(AST_Node) arena,AST_Node **start)
{
    *start=parseFile(node->list,0,arena);

    return *start==NULL;
    
}

