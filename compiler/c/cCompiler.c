#include "lex.h"
#include "parse.h"





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

