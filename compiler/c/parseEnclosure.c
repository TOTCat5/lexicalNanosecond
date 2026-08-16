#include "parseEnclosure.h"

typedef char enclosureStuff[2];

enclosureStuff findStartTable[]={
    {'(',')'},
    {'[',']'},
    {'{','}'},
};
size_t findFromStart(char startPonc,char endPonc)
{
    for(size_t i=0;i<_countof(findStartTable);++i)
    {
        if(
            (findStartTable[i][0]==startPonc)&&
            (findStartTable[i][1]==endPonc)
        )
        {
            return i;
        }
    }

    return FINDING_ERROR_VALUE;
}

size_t findEndEnclosure(const LexToken *tokens,char startPonc,size_t startI,size_t callerEndI)
{
    size_t enclosureCount=1;
        
    // first find end of code
    for(size_t i=startI;i<callerEndI;++i)
    {
        if(tokens[i].e==LEX_TOKEN_PONCTUATION)
        {
            char ponc=tokens[i].ponctuation;
            switch(ponc)
            {
                case '(':
                case '[':
                case '{':
                    enclosureCount++;
                break;

                case ')':
                case ']':
                case '}':
                    enclosureCount--;
                break;

                default:
                    continue;
            }

            if(enclosureCount==0)
            {
                size_t idx=findFromStart(startPonc,ponc);
                if(idx!=FINDING_ERROR_VALUE)
                {
                    // found end :D

                    return i;
                }
            }
        }
    }

    return FINDING_ERROR_VALUE;
}

getEnclosureResult getEnclosures(const LexToken *tokens,char startPonc,size_t startI,size_t callerEndI)
{
    getEnclosureResult result;
    listCreate(result.list.list,8);

    result.list.enclosurePonc=startPonc;

    size_t endIdx=FINDING_ERROR_VALUE;
    // check if has a start or is nothing
    if(startPonc!='\0')
    {
        endIdx=findEndEnclosure(tokens,startPonc,startI,callerEndI);
    }
    else
    {
        endIdx=callerEndI;
    }

    result.endIdx=endIdx;

    for(size_t i=startI;i<endIdx;++i)
    {
        if(tokens[i].e==LEX_TOKEN_PONCTUATION)
        {
            char ponc=tokens[i].ponctuation;

            switch(ponc)
            {
                case '(':
                {
                    getEnclosureResult t=getEnclosures(tokens,'(',i+1,endIdx);
                    i=t.endIdx;
                    EnclosureTreeToken token={
                        .isNewEnclosure=true,
                        .node=t.list
                    };
                    printf("%c\n",t.list.enclosurePonc);
                    listPushBack(result.list.list,token);
                    continue;
                }
                break;

                case '[':
                {
                    getEnclosureResult t=getEnclosures(tokens,'[',i+1,endIdx);
                    i=t.endIdx;
                    EnclosureTreeToken token={
                        .isNewEnclosure=true,
                        .node=t.list
                    };
                    printf("%c\n",t.list.enclosurePonc);
                    listPushBack(result.list.list,token);
                    continue;
                }
                break;
                
                case '{':
                {
                    getEnclosureResult t=getEnclosures(tokens,'{',i+1,endIdx);
                    i=t.endIdx;
                    EnclosureTreeToken token={
                        .isNewEnclosure=true,
                        .node=t.list
                    };
                    printf("%c\n",t.list.enclosurePonc);
                    listPushBack(result.list.list,token);
                    continue;
                }
                break;
            }

            
        }

        EnclosureTreeToken token={
            .isNewEnclosure=false,
            .token=tokens+i
        };
        listPushBack(result.list.list,token);
    }

    return result;
}
