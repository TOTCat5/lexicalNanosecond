#include "parse.h"

/*
    interLang is formed as "op_type(result,type(args)...)"

    the types are basically the normal types but in uppercase
    
    except for arguments,you'll have to "PUSH" and "POP" the values from the stack

    the ASSIGN uses the result as the destination while it's argument is the other variable

    you'll still have return a value

    "DEF funcName(args)" apart from rising my python senses wil define a function
    where as "END_FUNC" will signify the start of the end of the func code and the start of the cleanup code,as "END_OF_END_FUNC" will signify its end

    when calling a func,you shall use "FUNC(funcName)"

    a variable that will always exist is "returnValue" 

*/


// order,since order matters,it's the conversion ladder e.g:bool is tinier than int16 so it's converted to the other
// also if change the order or add something,check in interLang.c to synchronize with the others

enum InterLangTypeEnum;
typedef enum InterLangTypeEnum
{
    InterLangTypeBool,
    InterLangTypeInt16,
    InterLangTypeInt32,
    InterLangTypeInt64,
    InterLangTypeUint16,
    InterLangTypeUint32,
    InterLangTypeUint64,
    InterLangTypeFloat,
    InterLangTypeDouble,
    
    InterLangTypeNotAType=0xffffffff
} InterLangTypeEnum;


void generateInterLangCode(AST_Node *tree,FILE *outputFile);

