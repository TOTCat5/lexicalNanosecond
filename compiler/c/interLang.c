#include "interLang.h"

char *typeNames[]={
    "BOOL",
    "INT16",
    "INT32",
    "UINT16",
    "UINT32",
};

char *lnTypeNames[]={
    "bool",
    "int16",
    "int32",
    "uint16",
    "uint32"
};

// TODO:change code place
void fputLexToken(const LexToken *token,FILE *file)
{
    fwrite(token->str,1,token->strLen,file);
}

typedef struct InterLangVar
{
    union
    {
        const LexToken *nameToken;

        const char *nameStr;
    };

    InterLangTypeEnum e;

    bool included;
    bool pointToStr;

} InterLangVar;

typedef listType(InterLangVar) InterLangVarScope;




void cleanScope(InterLangVarScope *scope,FILE *file)
{
    while(listLength(*scope)!=0)
    {
        if(!listEnd(*scope).included)
        {
            fputs("POP_",file);
            fputs(typeNames[listEnd(*scope).e],file);
            fputs("(",file);
            
            fputLexToken(listEnd(*scope).nameToken,file);
            fputs(")\n",file);
        }

        listPopBack(*scope);
    }

    listDestroy(*scope);
}

void createScope(InterLangVarScope *scope)
{
    listCreate(*scope,4);
}

void pushToScope(InterLangVarScope *scope,InterLangVar *var,FILE *file)
{
    listPushBack(*scope,*var);
    if(!var->included)
    {
        fputs("PUSH_",file);
        fputs(typeNames[listEnd(*scope).e],file);
        fputs("(",file);
        if(!listEnd(*scope).pointToStr)
            fputLexToken(listEnd(*scope).nameToken,file);
        else
            fputs(listEnd(*scope).nameStr,file);
        fputs(")\n",file);
    }
}





InterLangTypeEnum lnTypeNameToInterLangTypeEnum(const LexToken *lnName)
{
    for(size_t i=0;i<sizeof(lnTypeNames)/sizeof(lnTypeNames[0]);++i)
    {
        if(isLexTokenEqualToStr(lnName,lnTypeNames[i]))
            return i;
    }

    return InterLangTypeNotAType;
}

InterLangVar *getInterLangVarInScopeFromAST_VarNode(AST_Node *node,InterLangVarScope *scope)
{
    for(size_t i=0;i<listLength(scope);++i)
    {
        if(scope[i]->nameToken==node->varNode.token)
        {
            return scope+i;
        }
    }

    return NULL;
}

void generateInterLangCodeInNewScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile);
void generateInterLangCodeInScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile);
InterLangVar *generateInterLangCodeInExpr(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile);

void generateInterLangCodeInScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{
    
    switch(tree->e)
    {
        case AST_NODE_DEC_VAR:
        {
            InterLangVar var={
                .nameToken=tree->decVarNode.nameToken,
                .e=lnTypeNameToInterLangTypeEnum(tree->decVarNode.typeNode->typeNode.token)
            };

            pushToScope(scope,&var,outputFile);
        }
        break;

        case AST_NODE_RETURN:
        {
            if(tree->returnNode.expr->e==AST_NODE_CONSTANT)
            {
                fputs("ASSIGN_INT32(returnValue,",outputFile);

                fputLexToken(tree->returnNode.expr->constantNode.token,outputFile);

                fputs(")\n",outputFile);
                fputs("RETURN_FUNC\n",outputFile);
            }
        }
        break;

            
        case AST_NODE_DEF_FUNC:
        {
            fputs("DEF ",outputFile);

            InterLangTypeEnum returnvalueTypeEnum=lnTypeNameToInterLangTypeEnum(tree->defFuncNode.typeNode->typeNode.token);
            fputs(typeNames[returnvalueTypeEnum],outputFile);

            fputs(" ",outputFile);

            fputLexToken(tree->defFuncNode.funcToken,outputFile);

            fputs("\n",outputFile);


            InterLangVarScope newScope;
            createScope(&newScope);

            const LexToken returnValue={
                .e=LEX_TOKEN_ID,
                .str="returnValue",
                .strLen=sizeof("returnValue")-1
            };
            InterLangVar var={
                .e=returnvalueTypeEnum,
                .nameToken=&returnValue,
                .included=true
            };
            pushToScope(scope,&var,outputFile);
            generateInterLangCodeInScope(tree->defFuncNode.code,&newScope,outputFile);

            fputs("END_FUNC\n",outputFile);
        }
        break;

        default:
            fprintf(stderr,"error,i don't deal with this bullshit");
        break;
            
    }
}

void generateInterLangCodeInNewScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{
    AST_Node *code=tree;

    while(code->e==AST_NODE_STATEMENT_LIST)
    {
        generateInterLangCodeInScope(code->statementListNode.node,scope,outputFile);

        code=code->statementListNode.next;
    }

    generateInterLangCodeInScope(code,scope,outputFile);
}

char numToHex[16]={
    '0','1','2','3','4','5','6','7','8','9',
    'a','b','c','d','e','f'
};

InterLangVar *generateInterLangCodeInExpr(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{
    static size_t creationIdx=0;

    #define getVarsExpr(vars) \
        if(tree->expressionNode.left->e==AST_NODE_EXPRESSION)\
        {\
            vars[0]=generateInterLangCodeInExpr(tree->expressionNode.left,scope,outputFile);\
        }\
        \
        if(tree->expressionNode.right->e==AST_NODE_EXPRESSION)\
        {\
            vars[1]=generateInterLangCodeInExpr(tree->expressionNode.left,scope,outputFile);\
        }\
        \
        if(vars[0]!=NULL)\
        {\
            if(tree->expressionNode.left->e==AST_NODE_VAR)\
            {\
                vars[0]=getInterLangVarInScopeFromAST_VarNode(tree->expressionNode.right->varNode.token,scope);\
            }\
        }\
        \
        if(vars[1]!=NULL)\
        {\
            if(tree->expressionNode.right->e==AST_NODE_VAR)\
            {\
                vars[1]=getInterLangVarInScopeFromAST_VarNode(tree->expressionNode.right->varNode.token,scope);\
            }\
        }

    switch(tree->expressionNode.op)
    {
        case AST_NODE_OPERATION_ADD:
        {
            InterLangVar *vars[2]={NULL,NULL};
            getVarsExpr(vars)


            char varName[256]={0};

            size_t tempIdx=creationIdx;

            for(size_t i=1;i<256;++i)
            {
                if(!tempIdx)
                {
                    break;
                }
                varName[i]=numToHex[tempIdx&0xf];

                tempIdx>>=4;
            }

            InterLangVar resultVar={
                .e=vars[0]->e,
                .nameStr=strdup(varName),
                .pointToStr=true
            };

            pushToScope(scope,&resultVar,outputFile);

            fputs("ADD_",outputFile);

            fputs(typeNames[vars[0]->e],outputFile);

            fputs("(",outputFile);
            fputs(resultVar.nameStr,outputFile);

            fputs(",",outputFile);
            fputs(vars[0]->nameStr,outputFile);
            fputs(",",outputFile);
            fputs(vars[1]->nameStr,outputFile);

            

            return &listEnd(*scope);
        }
        break;
    }

    #undef getVarsExpr
}


void generateInterLangCode(AST_Node *tree,FILE *outputFile)
{
    InterLangVarScope scope;
    createScope(&scope);

    generateInterLangCodeInNewScope(tree,&scope,outputFile);

    cleanScope(&scope,outputFile);

}
