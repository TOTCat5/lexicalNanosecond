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


typedef struct InterLangConstant
{
    InterLangTypeEnum type;
    
    union
    {
        bool        b;
        
        int8_t      i8;
        uint8_t     u8;

        int16_t     i16;
        uint16_t    u16;

        int32_t     i32;
        uint32_t    u32;

        int64_t     i64;
        uint64_t    u64;

        float f;

        double d;

        uint64_t branchInstructionIdx;
    };
} InterLangConstant;

typedef struct InterLangInstructionArg
{
    bool isConstant;

    union
    {
        InterLangConstant constant;
        InterLangVarId varId;
    };

} InterLangInstructionArg;


typedef struct InterLangFuncDeclaration InterLangFuncDeclaration;
typedef struct InterLangInstruction InterLangInstruction;
typedef struct InterLangFuncContext InterLangFuncContext;
typedef struct InterLangVar InterLangVar;


struct InterLangVar
{
    InterLangTypeEnum type;
    uint64_t id;
    // optional (for converting from ln variables to intLang)
    LexToken *tokenId;
};


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

    InterLangInstructionArg args[MAX_INSTRUCTION_ARGS];
};

struct InterLangFuncContext
{
    InterLangFuncDeclaration *declaration;

    listType(InterLangInstruction) instructions;
};

 


InterLangTypeEnum lnTypeNameToInterLangTypeEnum(const LexToken *lnName)
{
    for(size_t i=0;i<_countof(lnTypeNames);++i)
    {
        if(isLexTokenEqualToStr(lnName,lnTypeNames[i]))
            return i;
    }

    return InterLangTypeNotAType;
}




// without magic number
typedef struct InterLangFileHeader
{
    InterLangVersion fileVersion;
    union
    {
        uint64_t flags;

        struct
        {
            uint64_t endianess:1;
        };
        
    };
    
} InterLangFileHeader;


InterLangVarId idGenerator=0;

#define MAX_INTERLANG_VARIABLE_IN_CONTEXT (0x100)

listType(InterLangVar) allVars;

InterLangVarId varIdsInScope[MAX_INTERLANG_VARIABLE_IN_CONTEXT];
// keep track of when you enter a new scope
size_t contextAnchors[MAX_INTERLANG_VARIABLE_IN_CONTEXT];
size_t varsInScopeCount=0;

typedef listType(InterLangInstruction) InterLangInstructionOutput;


InterLangConstant getConstant(LexToken *token)
{
    bool isNeg=token->str[0]=='-';
    uint32_t intValue=0;
    double doubleValue=0;
    size_t numberLength=0;
    InterLangConstant resultConstant={
        .type=InterLangTypeInt32
    };
    for(size_t i=(size_t)isNeg;i<token->strLen;++i)
    {
        if(token->str[i]=='.')
        {
            resultConstant.type=InterLangTypeDouble;
            doubleValue=intValue;
            intValue=0;
            numberLength=0;
            continue;
        }

        numberLength++;

        intValue*=10;
        intValue+=token->str[i]-'0';
    }


    if(resultConstant.type==InterLangTypeDouble)
    {
        double dividor=1;
        for(size_t i=0;i<numberLength;++i)
        {
            dividor/=10.0;
        }

        resultConstant.d+=(double)intValue*dividor;
    }


    return resultConstant;
}


#define ID_NOT_FOUND UINT64_MAX

InterLangVarId getNewVar()
{
    listLength(allVars)++;
    listMemoryCheck(allVars);

    return idGenerator++;
}

InterLangVarId getVarId(LexToken *token)
{
    for(size_t reverseI=varsInScopeCount;reverseI!=0;--reverseI)
    {
        size_t i=reverseI-1;

        if(allVars[varIdsInScope[i]].tokenId==NULL)
        {
            continue;
        }

        if(isLexTokenEqual(allVars[varIdsInScope[i]].tokenId,token))
        {
            return i;
        }
    }

    return ID_NOT_FOUND;
}

InterLangInstructionArg getValue(AST_Node *tree,InterLangInstructionOutput *outputInstructions)
{
    switch(tree->e)
    {
        case AST_NODE_CONSTANT:
        {
            InterLangInstructionArg arg={
                .isConstant=true,
                .constant=getConstant(tree->constantNode.token)
            };

            return arg;
        }

        case AST_NODE_VAR:
        {
            InterLangInstructionArg arg={
                .isConstant=false,
                .varId=getVarId(tree->varNode.token)
            };

            return arg;
        }

        case AST_NODE_EXPRESSION:
        {
            InterLangInstructionArg arg=generateExpr(tree,outputInstructions);
            return arg;
        }
    }

    return (InterLangInstructionArg){
        .isConstant=true,
        .constant={
            .type=InterLangTypeNotAType
        }
    };
}

InterLangInstructionArg generateExpr(AST_Node *tree,InterLangInstructionOutput *outputInstructions)
{
    // TODO: check for branches

    InterLangVarId tempValueId=getNewVar();




    switch(tree->e)
    {
        case AST_NODE_OPERATION_NEG:
        {
            InterLangInstruction instruction={
                .instruction=InterLangInstructionNeg,
                .args={
                    getValue(tree->expressionNode.right,outputInstructions)
                }
            };
            listPushBack(*outputInstructions,instruction);
        }
        return;

        #define ARITHEMTIC_CASE(lnOp,intLangInstruction)\
        case lnOp:\
        {\
            InterLangInstruction instruction={\
                .instruction=intLangInstruction,\
                .args={\
                    getValue(tree->expressionNode.left,outputInstructions),\
                    getValue(tree->expressionNode.right,outputInstructions)\
                }\
            };\
            listPushBack(*outputInstructions,instruction);\
        }
        return;

        ARITHEMTIC_CASE(AST_NODE_OPERATION_ADD,InterLangInstructionAdd)
        ARITHEMTIC_CASE(AST_NODE_OPERATION_SUB,InterLangInstructionSub)
        ARITHEMTIC_CASE(AST_NODE_OPERATION_MUL,InterLangInstructionMul)
        ARITHEMTIC_CASE(AST_NODE_OPERATION_DIV,InterLangInstructionDiv)

    }


}


void generateInterLangCode(AST_Node *tree,FILE *outputFile)
{
    fwrite("intLang\0",8,1,outputFile);
    InterLangFileHeader fileHeader={
        .fileVersion=CURRENT_INTERLANG_VERSION,
        .endianess=__BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    };
    fwrite(&fileHeader,sizeof(fileHeader),1,outputFile);
    
}