#pragma once

#include "lex.h"


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

static const char *string_AST_NodeEnum(AST_NodeEnum e)
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

static const char *string_AST_NodeOperationEnum(AST_NodeOperationEnum e)
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


static size_t enclosureCheck(char ponctuation)
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

bool parse(listType(LexToken) tokenList,arenaType(AST_Node) arena,AST_Node **start);
