#include "lex.h"
#include "arena.h"


typedef struct EnclosureTreeNode EnclosureTreeNode;

typedef struct EnclosureTreeToken EnclosureTreeToken;

struct EnclosureTreeNode
{
    // '(','{',etc. ' ' is for no enclosure
    char enclosurePonc;

    listType(EnclosureTreeToken) list;

};

struct EnclosureTreeToken
{
    bool isNewEnclosure;
    union
    {

        EnclosureTreeNode node;

        const LexToken *token;
    };
};

#define hasEnclosureOfPonc(token,ponc) (token.isNewEnclosure&&ponc==token.node.enclosurePonc)

static const LexToken *getEnclosureTokenFunc(EnclosureTreeToken *token,size_t line,const char *fileName,const char *content)
{
    if(token->isNewEnclosure)
    {
        // do big error bad boo ah bleurgh

        return NULL;
    }
}

#define getEnclosureToken(token) (getEnclosureTokenFunc((token),__LINE__,__FILE__,#token))

typedef struct getEnclosureResult
{
    EnclosureTreeNode list;
    size_t endIdx;    
} getEnclosureResult;

getEnclosureResult getEnclosures(const LexToken *tokens,char startPonc,size_t startI,size_t callerEndI);
