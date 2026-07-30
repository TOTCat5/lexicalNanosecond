
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

            if test[0].startswith("ASSIGN_"):
                assignCommand=test[0][len("ASSIGN_"):]
                print(assignCommand)

                outFile.write("mov ")

                startArgsIdx=assignCommand.find("(")+1
                endArgsIdx  =assignCommand.find(")")

                assignArgs=assignCommand[startArgsIdx:endArgsIdx].split(",")

                if assignArgs[0]=="returnValue":
                    outFile.write("eax, ")

                outFile.write(assignArgs[1])

                outFile.write("\n")


                print(assignArgs)


            if test[0]=="RETURN_FUNC":
                outFile.write("ret\n")

            print(test)