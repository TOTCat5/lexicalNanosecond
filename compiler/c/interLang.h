#include "parse.h"

/*
    interLang is formed as "type_op(result,args...)"

    the types are basically the normal types but in uppercase
    
    except for arguments,you'll have to "PUSH" and "POP" the values from the stack

    the ASSIGN uses the result as the destination while it's argument is the other variable

    you'll still have return a value

    "def" apart from rising my python senses wil define a function
*/


typedef enum InterLangTypeEnum
{
    InterLangTypeInt16,
    InterLangTypeInt32,
    InterLangTypeUint16,
    InterLangTypeUint32
} InterLangTypeEnum;


void generateInterLangCode(AST_Node *tree,FILE *outputFile);

