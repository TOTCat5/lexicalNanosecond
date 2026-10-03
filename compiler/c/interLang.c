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


typedef struct InterLangFuncDeclaration InterLangFuncDeclaration;
typedef struct InterLangVarScope InterLangVarScope;
typedef struct InterLangInstruction InterLangInstruction;
typedef struct InterLangFuncContext InterLangFuncContext;

struct InterLangFuncDeclaration
{
    const LexToken *funcName;

    InterLangTypeEnum type;

    InterLangFuncContext *context;

    listType(InterLangVar) args;
};

struct InterLangInstruction
{
    InterLangInstructionEnum instruction;
    InterLangTypeEnum type;

    InterLangVar args[MAX_INSTRUCTION_ARGS];
};

struct InterLangFuncContext
{
    InterLangFuncDeclaration *declaration;

    listType(InterLangInstruction) instructions;
};


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

void putInterLangFuncDeclaration(InterLangFuncDeclaration *dec,FILE *file)
{
    fputs("DEF",file);
    fputs(typeNames[dec->type],file);
    fputLexToken(dec->funcName,file);

    for(size_t i=0;i<listLength(dec->args);++i)
    {
        fputs(typeNames[dec->args[i].e],file);
        fputs(" ",file);
        fputInterLangVar(dec->args+i,file);
    }
}

void putInterLangInstruction(InterLangInstruction *instruction,FILE *file)
{
    switch(instruction->type)
    {
        case InterLangInstructionAdd:
        
    }
}


void convertInterLangFuncContextToFile(InterLangFuncContext *funcContext,FILE *file)
{
    putInterLangFuncDeclaration(funcContext->declaration,file);

    puts(";");


    for(size_t i=0;i<listLength(funcContext->instructions);++i)
    {
        putInterLangInstruction(funcContext->instructions+i,file);
    }
}


InterLangTypeEnum lnTypeNameToInterLangTypeEnum(const LexToken *lnName)
{
    for(size_t i=0;i<_countof(lnTypeNames);++i)
    {
        if(isLexTokenEqualToStr(lnName,lnTypeNames[i]))
            return i;
    }

    return InterLangTypeNotAType;
}


