#include "interLang.h"

char *typeNames[]={
    "INT16",
    "INT32",
    "UINT16",
    "UINT32"
};

typedef struct InterLangVar
{
    char *name;

    InterLangTypeEnum e;
} InterLangVar;

typedef listType(InterLangVar) InterLangVarScope;

void cleanScope(InterLangVarScope *scope,FILE *file)
{
    while(listLength(*scope)!=0)
    {
        fputs(typeNames[(*scope)[listLength(*scope)-1].e],file);
        fputs("_POP(",file);
        fputs((*scope)[listLength(*scope)-1].name,file);
        fputs(")\n",file);

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
    fputs(typeNames[listEnd(*scope).e],file);
    fputs("_PUSH(",file);
    fputs(listEnd(*scope).name,file);
    fputs(")\n",file);
}


void generateInterLangCode(AST_Node *tree,FILE *outputFile)
{
    

}