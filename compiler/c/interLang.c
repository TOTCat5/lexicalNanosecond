#include "interLang.h"

char *typeNames[]={
    "BOOL",

    "INT8",
    "UINT8"
    
    "INT16",
    "UINT16",
    
    "INT32",
    "UINT32",
    
    "INT64"
    "UINT64",
    
    "FLOAT",
    "DOUBLE"
};

char *lnTypeNames[]={
    "bool",
    
    "int8",
    "uint8",
    
    "int16",
    "uint16",
    
    "int32",
    "uint32",
    
    "int64",
    "uint64",
    
    "float",
    "double"
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


typedef struct InterLangFunctionDeclaration
{
    const LexToken *funcName;

    InterLangTypeEnum e;

    listType(InterLangVar) args;
} InterLangFunctionDeclaration;

typedef listType(InterLangVar) InterLangVarScope;



void fputInterLangVar(InterLangVar *var,FILE *file)
{
    if(var->pointToStr)
    {
        fputs(var->nameStr,file);
    }
    else
    {
        fputLexToken(var->nameToken,file);
    }
}

// POP_... every variable created WITHOUT deleting it
void popScope(InterLangVarScope *scope,FILE *file)
{
    for(size_t i=0;i<listLength(*scope);++i)
    {
        if((*scope)[i].included)
        {
            continue;
        }

        fputs("POP_",file);
        fputs(typeNames[(*scope)[i].e],file);
        fputs("(",file);
        
        fputInterLangVar(&((*scope)[i]),file);
        fputs(")\n",file);
    }
}

// same as popScope but delete list afterward
void cleanScope(InterLangVarScope *scope,FILE *file)
{
    while(listLength(*scope)!=0)
    {
        if(!listEnd(*scope).included)
        {
            fputs("POP_",file);
            fputs(typeNames[listEnd(*scope).e],file);
            fputs("(",file);
            
            fputInterLangVar(&listEnd(*scope),file);
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

InterLangVar *getInterLangVarInScopeFromLexToken(const LexToken *token,InterLangVarScope *scope)
{
    for(size_t i=0;i<listLength(*scope);++i)
    {
        if(isLexTokenEqual((*scope)[i].nameToken,token))
        {
            return (*scope)+i;
        }
    }

    return NULL;
}

InterLangVar *getInterLangVarInScopeFromStr(char *str,InterLangVarScope *scope)
{
    for(size_t i=0;i<listLength(*scope);++i)
    {
        if(isLexTokenEqualToStr( (*scope)[i].nameToken,str))
        {
            return (*scope)+i;
        }
    }

    return NULL;
}

InterLangVar *getInterLangVarInScopeFromAST_VarNode(AST_Node *node,InterLangVarScope *scope)
{
    return getInterLangVarInScopeFromLexToken(node->varNode.token,scope);
}

InterLangTypeEnum getTypeOfConstantNodeLexToken(const LexToken *token)
{
    for(size_t i=0;i<token->strLen;++i)
    {
        if(token->str[i]=='.')
        {
            return InterLangTypeDouble;
        }
    }

    return InterLangTypeInt32;
}



listType(InterLangFunctionDeclaration) functionDeclarations=NULL;

InterLangFunctionDeclaration *getFunctionDeclarationFromLexToken(const LexToken *token)
{
    for(size_t i=0;i<listLength(functionDeclarations);++i)
    {
        if(isLexTokenEqual(functionDeclarations[i].funcName,token))
        {
            return functionDeclarations+i;
        }
    }

    return NULL;
}

InterLangVar returnValueVar={
    .e=InterLangTypeNotAType,
    .included=true,
    .nameStr="resultValue",
    .pointToStr=true
};

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
            InterLangVar *var=generateInterLangCodeInExpr(tree->returnNode.expr,scope,outputFile);

            fputs("ASSIGN_",outputFile);
            fputs(typeNames[__max(returnValueVar.e,var->e)],outputFile);
            fputs("(returnValue,",outputFile);
            fputInterLangVar(var,outputFile);
            fputs(")\n",outputFile);

            popScope(scope,outputFile);

            fputs("RETURN_FUNC\n",outputFile);

        }
        break;

            
        case AST_NODE_DEF_FUNC:
        {
            fputs("DEF ",outputFile);

            InterLangTypeEnum returnValueTypeEnum=lnTypeNameToInterLangTypeEnum(tree->defFuncNode.typeNode->typeNode.token);
            fputs(typeNames[returnValueTypeEnum],outputFile);

            fputs(" ",outputFile);

            fputLexToken(tree->defFuncNode.funcToken,outputFile);

            
            InterLangFunctionDeclaration funcDeclaration={
                .funcName=tree->defFuncNode.funcToken,
                .e=lnTypeNameToInterLangTypeEnum(tree->defFuncNode.typeNode->typeNode.token)
            };
            
            listCreate(funcDeclaration.args,4);

            InterLangVarScope newScope;
            createScope(&newScope);
            
            AST_Node *code=tree->defFuncNode.argList;
            while(code->e==AST_NODE_VALUE_LIST)
            {
                // should be AST_NODE_DEC_VAR
                if(code->valueListNode.value->e!=AST_NODE_DEC_VAR)
                {
                    return;
                }
                
                InterLangVar var={
                    .nameToken=code->valueListNode.value->decVarNode.nameToken,
                    .e=lnTypeNameToInterLangTypeEnum(code->valueListNode.value->decVarNode.typeNode->typeNode.token),
                    .included=true
                };
                listPushBack(funcDeclaration.args,var);
                
                pushToScope(&newScope,&var,outputFile);

                fputs(" ",outputFile);

                fputs(typeNames[var.e],outputFile);

                fputs(" ",outputFile);

                fputInterLangVar(&var,outputFile);


                code=code->valueListNode.next;
            }
            {
                InterLangVar var={
                    .nameToken=code->decVarNode.nameToken,
                    .e=lnTypeNameToInterLangTypeEnum(tree->decVarNode.typeNode->typeNode.token),
                    .included=true
                };
                listPushBack(funcDeclaration.args,var);

                pushToScope(&newScope,&var,outputFile);
                
                fputs(" ",outputFile);

                fputs(typeNames[var.e],outputFile);

                fputs(" ",outputFile);

                fputInterLangVar(&var,outputFile);
            }

            listPushBack(functionDeclarations,funcDeclaration);
            
            fputs("\n",outputFile);

            returnValueVar.e=returnValueTypeEnum;
            
            generateInterLangCodeInScope(tree->defFuncNode.code,&newScope,outputFile);

            fputs("END_FUNC\n",outputFile);

            listDestroy(newScope);

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

InterLangVar *generateInterLangCodeForExprNode(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{
    static size_t creationIdx=0;

    #define getVarsExpr(vars) \
        if(\
            tree->expressionNode.left->e==AST_NODE_EXPRESSION||\
            tree->expressionNode.left->e==AST_NODE_CALLING_FUNC\
        )\
        {\
            vars[0]=generateInterLangCodeInExpr(tree->expressionNode.left,scope,outputFile);\
        }\
        \
        if(\
            tree->expressionNode.right->e==AST_NODE_EXPRESSION||\
            tree->expressionNode.right->e==AST_NODE_CALLING_FUNC\
        )\
        {\
            vars[1]=generateInterLangCodeInExpr(tree->expressionNode.right,scope,outputFile);\
        }\
        \
        if(vars[0]!=NULL)\
        {\
            if(tree->expressionNode.left->e==AST_NODE_VAR)\
            {\
                vars[0]=getInterLangVarInScopeFromAST_VarNode(tree->expressionNode.left,scope);\
            }\
        }\
        \
        if(vars[1]!=NULL)\
        {\
            if(tree->expressionNode.right->e==AST_NODE_VAR)\
            {\
                vars[1]=getInterLangVarInScopeFromAST_VarNode(tree->expressionNode.right,scope);\
            }\
        }

    #define handleArithemticOp(opName)\
        InterLangVar *vars[2]={NULL,NULL};\
        getVarsExpr(vars)\
        char varName[256]={0};\
        varName[0]='t';\
        size_t tempIdx=++creationIdx;\
        for(size_t i=1;i<256;++i)\
        {\
            if(!tempIdx)\
            {\
                break;\
            }\
            varName[i]=numToHex[tempIdx&0xf];\
            tempIdx>>=4;\
        }\
        InterLangVar tempForConstantVar[2]={0};\
        if(!vars[0])\
        {\
            if(tree->expressionNode.left->e!=AST_NODE_CONSTANT)\
            {\
                printf("tree->expressionNode.left->e!=AST_NODE_CONSTANT\n");\
            }\
            tempForConstantVar[0].e=getTypeOfConstantNodeLexToken(tree->expressionNode.left->constantNode.token);\
            tempForConstantVar[0].nameToken=tree->expressionNode.left->constantNode.token;\
            vars[0]=tempForConstantVar+0;\
        }\
        if(!vars[1])\
        {\
            if(tree->expressionNode.right->e!=AST_NODE_CONSTANT)\
            {\
                printf("tree->expressionNode.right->e!=AST_NODE_CONSTANT\n");\
            }\
            tempForConstantVar[1].e=getTypeOfConstantNodeLexToken(tree->expressionNode.right->constantNode.token);\
            tempForConstantVar[1].nameToken=tree->expressionNode.right->constantNode.token;\
            vars[1]=tempForConstantVar+1;\
        }\
        InterLangVar resultVar={\
            .e=vars[0]->e,\
            .nameStr=strdup(varName),\
            .pointToStr=true\
        };\
        pushToScope(scope,&resultVar,outputFile);\
        fputs(opName "_",outputFile);\
        fputs(typeNames[vars[0]->e],outputFile);\
        fputs("(",outputFile);\
        fputs(resultVar.nameStr,outputFile);\
        fputs(",",outputFile);\
        fputInterLangVar(vars[0],outputFile);\
        fputs(",",outputFile);\
        fputInterLangVar(vars[1],outputFile);\
        fputs(")\n",outputFile);\
        return &listEnd(*scope);\

    switch(tree->expressionNode.op)
    {
        case AST_NODE_OPERATION_ADD:
        {
            handleArithemticOp("ADD")
        }
        break;

        case AST_NODE_OPERATION_SUB:
        {
            handleArithemticOp("SUB")
            // InterLangVar *vars[2]={((void *)0),((void *)0)};
            // if(
            //     tree->expressionNode.left->e==AST_NODE_EXPRESSION||
            //     tree->expressionNode.left->e==AST_NODE_CALLING_FUNC
            // )
            // {
            //     vars[0]=generateInterLangCodeInExpr(tree->expressionNode.left,scope,outputFile);
            // }
            // if(
            //     tree->expressionNode.right->e==AST_NODE_EXPRESSION||
            //     tree->expressionNode.right->e==AST_NODE_CALLING_FUNC
            // )
            // {
            //     vars[1]=generateInterLangCodeInExpr(tree->expressionNode.left,scope,outputFile);
            // }
            // if(vars[0]!=((void *)0))
            // {
            //     if(tree->expressionNode.left->e==AST_NODE_VAR)
            //     {
            //         vars[0]=getInterLangVarInScopeFromAST_VarNode(tree->expressionNode.left,scope);
            //     }
            // }
            // if(vars[1]!=((void *)0))
            // {
            //     if(tree->expressionNode.right->e==AST_NODE_VAR)
            //     {
            //         vars[1]=getInterLangVarInScopeFromAST_VarNode(tree->expressionNode.right,scope);
            //     }
            // }
            // char varName[256]={0};
            // varName[0]='t';
            // size_t tempIdx=++creationIdx;
            // for(size_t i=1;i<256;++i)
            // {
            //     if(!tempIdx)
            //     {
            //         break;
            //     }
            //     varName[i]=numToHex[tempIdx&0xf];
            //     tempIdx>>=4;
            // }
            // InterLangVar tempForConstantVar[2]={0};
            // if(!vars[0])
            // {
            //     if(tree->expressionNode.left->e!=AST_NODE_CONSTANT)
            //     {
            //         printf("tree->expressionNode.left->e!=AST_NODE_CONSTANT\n");
            //     }
            //     tempForConstantVar[0].e=getTypeOfConstantNodeLexToken(tree->expressionNode.left->constantNode.token);
            //     tempForConstantVar[0].nameToken=tree->expressionNode.left->constantNode.token;
            //     vars[0]=tempForConstantVar+0;
            // }
            // if(!vars[1])
            // {
            //     if(tree->expressionNode.right->e!=AST_NODE_CONSTANT)
            //     {
            //         printf("tree->expressionNode.right->e!=AST_NODE_CONSTANT\n");
            //     }
            //     tempForConstantVar[1].e=getTypeOfConstantNodeLexToken(tree->expressionNode.right->constantNode.token);
            //     tempForConstantVar[1].nameToken=tree->expressionNode.right->constantNode.token;
            //     vars[1]=tempForConstantVar+1;
            // }
            // InterLangVar resultVar={
            //     .e=vars[0]->e,
            //     .nameStr=strdup(varName),
            //     .pointToStr=true
            // };
            // pushToScope(scope,&resultVar,outputFile);
            // fputs("SUB" "_",outputFile);
            // fputs(typeNames[vars[0]->e],outputFile);
            // fputs("(",outputFile);
            // fputs(resultVar.nameStr,outputFile);
            // fputs(",",outputFile);
            // fputInterLangVar(vars[0],outputFile);
            // fputs(",",outputFile);
            // fputInterLangVar(vars[1],outputFile);
            // fputs(")\n",outputFile);
            // return &((*scope)[(((_d_ListHeader_ *)(*scope)-1)->length)-1]);
        }
        break;
        
        case AST_NODE_OPERATION_MUL:
        {
            handleArithemticOp("MUL")
        }
        break;

        case AST_NODE_OPERATION_DIV:
        {
            handleArithemticOp("DIV")
        }
        break;
    }

    #undef handleArithemticOp
    #undef getVarsExpr
}



InterLangVar *generateInterLangCodeForCallingFuncNode(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{
    if(tree->callingFuncNode.func->e!=AST_NODE_VAR)
    {
        return NULL;
    }

    InterLangFunctionDeclaration *declaration=getFunctionDeclarationFromLexToken(tree->callingFuncNode.func->varNode.token);

    fputs("CALLFUNC ",outputFile);

    fputLexToken(declaration->funcName,outputFile);

    for(size_t i=0;i<listLength(declaration->args);++i)
    {
        fputs(" ",outputFile);
        fputs(typeNames[declaration->args[i].e],outputFile);
        fputs(" ",outputFile);
        fputInterLangVar(declaration->args+i,outputFile);
    }

    fputs("\n",outputFile);

    returnValueVar.e=declaration->e;
    
    return &returnValueVar;
}

InterLangVar *generateInterLangCodeInExpr(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{

    if(tree->e==AST_NODE_EXPRESSION)
    {
        return generateInterLangCodeForExprNode(tree,scope,outputFile);
    }
    else if(tree->e==AST_NODE_CALLING_FUNC)
    {
        return generateInterLangCodeForCallingFuncNode(tree,scope,outputFile);
    }
    else if(tree->e==AST_NODE_VAR)
    {
        return getInterLangVarInScopeFromAST_VarNode(tree,scope);
    }

    return NULL;
}


void generateInterLangCode(AST_Node *tree,FILE *outputFile)
{
    listCreate(functionDeclarations,16);

    InterLangVarScope scope;
    createScope(&scope);

    generateInterLangCodeInNewScope(tree,&scope,outputFile);

    cleanScope(&scope,outputFile);

}
