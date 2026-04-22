#include <stdio.h>
#include "list.h"
#include <stdint.h>


int main(int argc,char *argv[])
{
    FILE *outFile=fopen("out/cCompiler.asm","wb");

    fprintf(outFile,
        "extern ExitProcess\n"
        "xor ecx, ecx       ; exit code = 0\n"
        "call ExitProcess"
    );
}

