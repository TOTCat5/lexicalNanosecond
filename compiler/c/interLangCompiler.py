
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

#              varName, stackPtr, size
pushList:list[(str,     int,      int)]=[]

def pushVar(varName:str,typeName:str):
    stackPtr:int=0
    if len(pushList)!=0:
        stackPtr=pushList[-1][1]+pushList[-1][2]

    pushList.append((varName,stackPtr,typeToSize[typeName]))

def getVarSubtract(varName:str)->int:
    for i in reversed(pushList):
        if varName==i[0]:
            return (pushList[-1][1]+pushList[-1][2])-i[1]

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
            outFile.write("mov ")
            if commandArgs[0]=="returnValue":
                outFile.write(registers["eax"][typeName]+", [esp+"+str(getVarSubtract(commandArgs[1]))+"]")
            else:
                tempReg=registers["ebx"][typeName]
                outFile.write(tempReg+", [esp"+str(getVarSubtract(commandArgs[1]))+"]")
                outFile.write("[esp"+str(getVarSubtract(commandArgs[1]))+"], "+tempReg)


            outFile.write("\n")



        def addCommand(commandArgs:list[str],typeName:str):
            if commandArgs[0]==commandArgs[1]:
                tempReg=registers["ebx"][typeName]
                result:str=(
                    "mov "+tempReg+",[esp+"+str(getVarSubtract(commandArgs[0]))+"]\n"+
                    "add"+tempReg+ ",[esp+"+str(getVarSubtract(commandArgs[2]))+"]\n"
                )

                print(result)
                                
                outFile.write(result)

                return

            tempReg0=registers["ebx"][typeName]
            tempReg1=registers["ecx"][typeName]

            result:str=(
                "mov "+tempReg0+",[esp+"+str(getVarSubtract(commandArgs[1]))+"]\n"
                "mov "+tempReg1+",[esp+"+str(getVarSubtract(commandArgs[2]))+"]\n"
                "add "+tempReg0+","+tempReg1+"\n"
                "mov [esp+"+str(getVarSubtract(commandArgs[0]))+"],"+tempReg0+"\n"
            )

            print(result)

            outFile.write(result)

        def popCommand(commandArgs:list[str],typeName:str):
            assert(len(commandArgs)==1)

            assert(pushList[-1][0]==commandArgs[0])

            outFile.write("add esp,"+str(pushList[-1][2])+"\n")
            pushList.pop()


            
            


        commands:list[(function,str)]=[
            (pushCommand,   "PUSH"),
            (assignCommand, "ASSIGN"),
            (addCommand,    "ADD"),
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

                    pushVar(argName,funcArgs[i*2])

                    funcContext.append(argName)

                    i+=1

                print(funcArgs)

                outFile.write("push ebp\nmov ebp,esp\n")

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
                assert(funcContext[-1]==pushList[-1][0])

                outFile.write("mov esp,ebp\npop ebp\nret\n")

                continue



            if test[0]=="END_FUNC":
                continue


            print(test)


            print("\033[91munhandled case: "+i)
            print        ("                ",end="")
            for i in range(len(i)):
                print("~",end="")

            print("\033[0m")
            outFile.flush()
            exit(-1)