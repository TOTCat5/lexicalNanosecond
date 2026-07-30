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
    const LexToken *name;

    InterLangTypeEnum e;

    bool included;
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
            
            fputLexToken(listEnd(*scope).name,file);
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
        fputLexToken(listEnd(*scope).name,file);
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

void generateInterLangCodeInNewScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile);
void generateInterLangCodeInScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile);


void generateInterLangCodeInScope(AST_Node *tree,InterLangVarScope *scope,FILE *outputFile)
{
    switch(tree->e)
    {
        case AST_NODE_DEC_VAR:
        {
            InterLangVar var={
                .name=tree->decVarNode.nameToken,
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
                .name=&returnValue,
                .included=true
            };
            pushToScope(scope,&var,outputFile);
            generateInterLangCodeInScope(tree->defFuncNode.code,&newScope,outputFile);
        }
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


void generateInterLangCode(AST_Node *tree,FILE *outputFile)
{
    InterLangVarScope scope;
    createScope(&scope);

    generateInterLangCodeInNewScope(tree,&scope,outputFile);

    cleanScope(&scope,outputFile);

}
