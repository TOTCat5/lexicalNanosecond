
registers={
    "eax":{
        "INT32":  "eax",
        "UINT32": "eax",
        "INT16":   "ax",
        "UINT16":  "ax"
    },
    "ebx":{
        "INT32":  "ebx",
        "UINT32": "ebx",
        "INT16":   "bx",
        "UINT16":  "bx"
    },
    "ecx":{
        "INT32":  "ecx",
        "UINT32": "ecx",
        "INT16":   "cx",
        "UINT16":  "cx"
    },
    "edx":{
        "INT32":  "edx",
        "UINT32": "edx",
        "INT16":   "dx",
        "UINT16":  "dx"
    }
}

typeToSize={
    "INT32":4,
    "UINT32":4,
    "INT16":2,
    "UINT16":2
}

#              varName, stackPtr, size, funcArg
pushList:list[(str,     int,      int,  bool   )]=[]
endFuncArgIdx:int=0 

PUSH_LIST_VAR_NAME=0
PUSH_LIST_STACK_PTR=1
PUSH_LIST_SIZE=2
PUSH_LIST_FUNC_ARG=3

def pushVar(varName:str,typeName:str):
    stackPtr:int=0
    if len(pushList)!=0 and (not pushList[-1][PUSH_LIST_FUNC_ARG]):
        stackPtr=pushList[-1][PUSH_LIST_STACK_PTR]+pushList[-1][PUSH_LIST_SIZE]

    pushList.append((varName,stackPtr,typeToSize[typeName],False))

def pushFuncArg(argName:str,typeName:str):
    stackPtr:int=0
    if len(pushList)!=0:
        stackPtr=pushList[-1][PUSH_LIST_STACK_PTR]+pushList[-1][PUSH_LIST_SIZE]

    pushList.append((argName,stackPtr,typeToSize[typeName],True))
    endFuncArgIdx=len(pushList)
    

def getVarSubStr(varName:str,typeName:str)->str:
    if varName=="returnValue":
        return registers["eax"][typeName]
    for i in reversed(pushList):
        if varName==i[PUSH_LIST_VAR_NAME]:
            if i[PUSH_LIST_FUNC_ARG]:
                #                  8 for return pointer and the stack frame
                return "[ebp+"+str(8+i[PUSH_LIST_STACK_PTR]-pushList[endFuncArgIdx][PUSH_LIST_STACK_PTR]+i[PUSH_LIST_SIZE])+"]"
            else:
                return "[ebp-"+str(i[PUSH_LIST_STACK_PTR]-pushList[endFuncArgIdx][PUSH_LIST_STACK_PTR])+"]"

    print("error unknown variable: "+varName)

# context to handle functions
funcContext:list[str]=[]
#           argList



with open("compiler/out/cCompiler.asm","w") as outFile:
    with open("compiler/out/cCompiler.intLang","r") as inFile:

        
        def pushCommand(commandArgs:list[str],typeName:str):
            assert(len(commandArgs)==1)

            pushVar(commandArgs[0],typeName)

            outFile.write("sub esp,"+str(typeToSize[typeName])+"\n")

        def assignCommand(commandArgs:list[str],typeName:str):
            assert(len(commandArgs)==2)
            outFile.write("mov ")

            if commandArgs[0]=="returnValue":
                outFile.write(registers["eax"][typeName]+", "+getVarSubStr(commandArgs[1],typeName))
            else:
                if commandArgs[1]=="returnValue":
                                    outFile.write(getVarSubStr(commandArgs[0],typeName)+",eax")
                else:
                    tempReg=registers["ebx"][typeName]
                    outFile.write(tempReg+", "+getVarSubStr(commandArgs[1],typeName)+"\n")
                    outFile.write("mov "+getVarSubStr(commandArgs[0],typeName)+","+tempReg)


            outFile.write("\n")



        def addCommand(commandArgs:list[str],typeName:str):
            assert(len(commandArgs)==3)

            if commandArgs[0]==commandArgs[1]:
                tempReg=registers["ebx"][typeName]
                result:str=(
                    "mov "+tempReg+","+getVarSubStr(commandArgs[0],typeName)+"\n"+
                    "add"+tempReg+ ","+getVarSubStr(commandArgs[2],typeName)+"\n"
                )

                print(result)
                                
                outFile.write(result)

                return

            tempReg0=registers["ebx"][typeName]
            tempReg1=registers["ecx"][typeName]

            result:str=(
                "mov "+tempReg0+", "+getVarSubStr(commandArgs[1],typeName)+"\n"
                "mov "+tempReg1+", "+getVarSubStr(commandArgs[2],typeName)+"\n"
                "add "+tempReg0+", "+tempReg1+"\n"
                "mov "+getVarSubStr(commandArgs[0],typeName)+", "+tempReg0+"\n"
            )

            print(result)

            outFile.write(result)

        def subCommand(commandArgs:list[str],typeName:str):
            assert(len(commandArgs)==3)
            if commandArgs[0]==commandArgs[1]:
                tempReg=registers["ebx"][typeName]
                result:str=(
                    "mov "+tempReg+","+getVarSubStr(commandArgs[0],typeName)+"\n"+
                    "sub"+tempReg+ ","+getVarSubStr(commandArgs[2],typeName)+"\n"
                )

                print(result)
                                
                outFile.write(result)

                return

            tempReg0=registers["ebx"][typeName]
            tempReg1=registers["ecx"][typeName]

            result:str=(
                "mov "+tempReg0+","+getVarSubStr(commandArgs[1],typeName)+"\n"
                "mov "+tempReg1+","+getVarSubStr(commandArgs[2],typeName)+"\n"
                "sub "+tempReg0+","+tempReg1+"\n"
                "mov "+getVarSubStr(commandArgs[0],typeName)+","+tempReg0+"\n"
            )

            print(result)

            outFile.write(result)
            
        def popCommand(commandArgs:list[str],typeName:str):
            # pass
            assert(len(commandArgs)==1)

            assert(pushList[-1][0]==commandArgs[0])

            # outFile.write("add esp,"+str(pushList[-1][PUSH_LIST_SIZE])+"\n")
            # pushList.pop()
            # possibly removeable since "mov esp, ebp" should be resetting anything esp left


            
            


        commands:list[(function,str)]=[
            (pushCommand,   "PUSH"),
            (assignCommand, "ASSIGN"),
            (addCommand,    "ADD"),
            (subCommand,    "SUB"),
            (popCommand,    "POP")
        ]

        lines=inFile.readlines()

        # last character of last line
        if lines[-1][-1]!='\n':
            print("error lines[-1][-1]!='\\n'")
            exit(-1)

        for i in lines:
            i=i[:-1]
            print(i)

            test=i.split(" ")

            if test[0]=="DEF":
                funcArgs:list[str]=test[3:]
                funcName:str=test[2]

                outFile.write(funcName+":\n")

                i:int=0
                while i<len(funcArgs)/2:

                    argName:str=funcArgs[i*2+1]

                    pushFuncArg(argName,funcArgs[i*2])

                    funcContext.append(argName)

                    i+=1

                print(funcContext)

                outFile.write("push ebp\nmov ebp,esp\n")

                continue

            if test[0]=="CALLFUNC":

                funcArgs:list[str]=test[2:]
                funcName:str=test[1]


                print(funcArgs)

                argStackSize:int=0

                i:int=0
                while i<len(funcArgs)/2:

                    argName:str=funcArgs[i*2+1]
                    argTypeName:str=funcArgs[i*2]

                    outFile.write("sub esp,"+str(typeToSize[argTypeName])+"\n")
                    outFile.write("mov "+registers["ebx"][argTypeName]+","+getVarSubStr(argName,argTypeName)+"\n")
                    outFile.write("mov [esp+"+str(typeToSize[argTypeName])+"],"+registers["ebx"][argTypeName]+"\n")

                    argStackSize+=typeToSize[argTypeName]

                    i+=1

                outFile.write("call "+funcName+"\n")

                outFile.write("add esp,"+str(argStackSize)+"\n")
                
                continue


            found:bool=False
            for x in commands:
                if test[0].startswith(x[1]+"_"):
                    cmd=test[0][len(x[1]+"_"):]

                    startArgsIdx=cmd.find("(")
                    endArgsIdx  =cmd.find(")")
    
                    typeName=cmd[:startArgsIdx]

                    commandArgs=cmd[startArgsIdx+1:endArgsIdx].split(",")
                    

                    x[0](commandArgs,typeName)

                    found=True

                    break

            if found:
                continue

            


            if test[0]=="RETURN_FUNC":
                
                outFile.write("mov esp,ebp\npop ebp\nret\n")


                

                continue



            if test[0]=="END_FUNC":
                # remove func args in pushList
                for i in range(len(funcContext)):
                    pushList.pop()


                funcContext.clear()
                continue


            print(test)


            print("\033[91munhandled case: "+i)
            print        ("                ",end="")
            for i in range(len(i)):
                print("~",end="")

            print("\033[0m")
            outFile.flush()
            exit(-1)