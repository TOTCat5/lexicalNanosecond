
registers={"eax":{"INT32":"eax","UINT32":"eax","INT16":"ax","UINT16":"ax"}}


with open("compiler/out/cCompiler.asm","w") as outFile:
    with open("compiler/out/cCompiler.intLang","r") as inFile:
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
                outFile.write(test[2]+":\n")
                continue

            if test[0].startswith("ASSIGN_"):
                assignCommand=test[0][len("ASSIGN_"):]
                print(assignCommand)

                outFile.write("mov ")


                startArgsIdx=assignCommand.find("(")
                endArgsIdx  =assignCommand.find(")")

                typeName=assignCommand[:startArgsIdx]

                assignArgs=assignCommand[startArgsIdx+1:endArgsIdx].split(",")

                if assignArgs[0]=="returnValue":
                    outFile.write(registers["eax"][typeName]+", ")

                outFile.write(assignArgs[1])

                outFile.write("\n")


                print(assignArgs)

                continue


            if test[0]=="RETURN_FUNC":
                outFile.write("ret\n")

                continue


            if test[0]=="END_FUNC":
                continue

            print(test)


            print("unhandled case:"+i)
            exit(-1)