import sys

#takes complement of the given binary string
def takeComplement(bin):
    #complement of zero is zero
    if int(bin,2) == 0:
        return bin

    #flip the bits of the input string
    flipped = ""
    for char in bin:
        if char == "1":
            flipped = flipped + "0"
        elif char == "0":
            flipped = flipped + "1"

    #add 1 to the flipped string to get complement
    n=addBin(flipped,"0000000000000001")
    return n

#takes the not of the given binary string ie flips the bits
def takeNot(bin):
    notString = ""
    for char in bin:
        if char == "1":
            notString = notString + "0"
        elif char == "0":
            notString = notString + "1"
    return notString

#finds the addition of two binary strings
def addBin(bin1,bin2):

    res = ""
    carry = 0       #carry is zero at first
    #starting from the most-right bits do the the addition
    for i in range(15,-1,-1):
        num1 = int(bin1[i],2)
        num2 = int(bin2[i],2)
        if (num1+num2+carry) % 2 == 1:
            res = res + "1"
        else:
            res = res + "0"

        if (num1+num2+carry) // 2 == 1:
            carry = 1
        else:
            carry = 0

    #after the addition if there is carry add it aswell
    if carry == 1:
        res = res + "1"
    #reverse the string so we will get the actual result
    res = res[::-1]
    return res

    res = ""
    carry = 0
    for i in range(3,-1,-1):
        num1 = int(bin1[i],2)
        num2 = int(bin2[i],2)
        if (num1+num2+carry) % 2 == 1:
            res = res + "1"
        else:
            res = res + "0"
        if (num1+num2+carry) // 2 == 1:
            carry = 1
        else:
            carry = 0

    if carry == 1:
        res = res + "1"
    res = res[::-1]
    return res

#consult this dictionary when converting given .bin file to binary
hexToBinary = {
    "0":"0000",
    "1":"0001",
    "2":"0010",
    "3":"0011",
    "4":"0100",
    "5":"0101",
    "6":"0110",
    "7":"0111",
    "8":"1000",
    "9":"1001",
    "A":"1010",
    "B":"1011",
    "C":"1100",
    "D":"1101",
    "E":"1110",
    "F":"1111"
}

#registers and their initial values, will be updated during execution
registers = {
    "A":"0000000000000000",
    "B":"0000000000000000",
    "C":"0000000000000000",
    "D":"0000000000000000",
    "E":"0000000000000000",
    "S":2**16-1,
    "PC":0
}

#distinct encodings for each register
registerEncodings = {
    "0000000000000000":"PC",
    "0000000000000001":"A",
    "0000000000000010":"B",
    "0000000000000011":"C",
    "0000000000000100":"D",
    "0000000000000101":"E",
    "0000000000000110":"S"
}

fileIn = open(sys.argv[1],"r")      #file to be read
fileOut = open(sys.argv[1][:-3]+"txt","w")    #file to be written

ZF=CF=SF = None     #Flags

memory = []         #memory

#all memory cells have the initial value 0
for i in range(0,64*1024):
    memory.append("00000000")

i = 0
#reads .bin file line by line
for line in fileIn:
    binString = ""
    for char in line:
        if (char == '\n'):
            continue
        #convert it to binary using dictionary defined above
        binString += hexToBinary[char]
    #since each memory cell is 8 bits divide the binary string into 3 parts
    memory[i] = binString[0:8]
    memory[i+1] = binString[8:16]
    memory[i+2] = binString[16:]
    i=i+3

fileIn.close()

#executes the binary commands and do the necessary operations
while True:
    #get the instruction in memory pointed by PC
    currentInstruction = memory[registers["PC"]]

    opcode = currentInstruction[0:6]        #first 6 bits are opcode
    opcode = int(opcode,2)
    addressMode = currentInstruction[6:8]   #last 2 bits are addressing mode

    opPart1 = memory[registers["PC"]+1]     #operand is found in two consecutive memory cell
    opPart2 = memory[registers["PC"]+2]
    operand = opPart1 + opPart2

    #end the process
    if opcode == 1:     #HALT
        fileOut.close()
        break

    #assigns the value of the operand to register A
    elif opcode == 2:   #LOAD

        if addressMode == "00" :
            registers["A"] = operand
        elif addressMode == "01":               #find which register and assign
            reg = registerEncodings[operand]
            registers["A"] = registers[reg]
        elif addressMode== "10":                #find which register and find the operand in given address
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg],2)
            registers["A"] = memory[memoryAddress] + memory[memoryAddress+1]
        elif addressMode == "11":               #go to cell and make the assigment
            memoryAddress = int(operand, 2)
            registers["A"] = memory[memoryAddress] + memory[memoryAddress + 1]

    #assign the value of register A to operand
    elif opcode == 3:   #STORE

        if addressMode == "01":
            reg = registerEncodings[operand]
            registers[reg] = registers["A"]
        elif addressMode == "10":                   #when assigning to memory divide it into two to make it fit
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg],2)
            memory[memoryAddress] = registers["A"][0:8]
            memory[memoryAddress + 1] = registers["A"][8:]
        elif addressMode == "11":
            memoryAddress = int(operand,2)
            memory[memoryAddress] = registers["A"][0:8]
            memory[memoryAddress + 1] = registers["A"][8:]

    #add the operand to register A and store the result there
    #since the remaining if blocks are nearly the same we will only comment when there is a difference
    elif opcode == 4:   #ADD

        actualOperand = ""          #find the actual operand considering the possible addressing modes
        if addressMode == "00":
            actualOperand = operand

        elif  addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg],2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        #once you find the actual data simply make the calculation
        result = addBin(registers["A"], actualOperand)

        #set the flags after the computation accordingly
        CF = SF = ZF = 0
        if len(result) == 17:       #16 bit addition might end up with 17 bits in this case there is carry
            CF = 1
            result = result[1:]
        if result[0] == "1":        #if most-significant bit is 1 sign flag is 1
            SF = 1
        if int(result,2) == 0:      # if result is zero zero flag is true
            ZF = 1

        registers["A"] = result

    #subtract the operand from register A and store the result there
    elif opcode == 5:   #SUB

        actualOperand =""
        if addressMode == "00":
            actualOperand = operand

        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        # implement A-X as A+(-X) so first take the complement
        negatedOperand = takeComplement(actualOperand)
        #then make the addition
        result = addBin(registers["A"], negatedOperand)

        CF = SF = ZF = 0
        if len(result) == 17:
            CF = 1
            result = result[1:]
        if result[0] == "1":
            SF = 1
        if int(result, 2) == 0:
            ZF = 1

        registers["A"]=result

    #increment the operand
    elif opcode == 6:   #INC

        actualOperand =""
        if addressMode == "00":
            actualOperand = operand

        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        #incrementing is the same as adding 1
        result = addBin(actualOperand,"0000000000000001")

        CF = SF = ZF = 0
        if len(result) == 17:
            CF = 1
            result = result[1:]
        if result[0] == "1":
            SF = 1
        if int(result, 2) == 0:
            ZF = 1

        #update the operands value wherever it is stored
        if addressMode == "01":
            registers[reg] = result
        elif addressMode == "10":
            memory[memoryAddress] = result[0:8]
            memory[memoryAddress + 1] = result[8:]
        elif addressMode == "11":
            memory[memoryAddress] = result[0:8]
            memory[memoryAddress + 1] = result[8:]

    #decrement the operand
    elif opcode == 7:   #DEC

        actualOperand = ""
        if addressMode == "00":
            actualOperand = operand

        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        #decrementing is same as subtracting 1 and in our case same as adding -1
        result = addBin(actualOperand,"1111111111111111")

        CF = SF = ZF = 0
        if len(result) == 17:
            CF = 1
            result = result[1:]
        if result[0] == "1":
            SF = 1
        if int(result, 2) == 0:
            ZF = 1

        if addressMode == "01":
            registers[reg] = result
        elif addressMode == "10":
            memory[memoryAddress] = result[0:8]
            memory[memoryAddress + 1] = result[8:]
        elif addressMode == "11":
            memory[memoryAddress] = result[0:8]
            memory[memoryAddress + 1] = result[8:]

    #find the XOR of the value in register A and operand
    elif opcode == 8:   #XOR

        actualOperand = ""
        if addressMode == "00":
            actualOperand = operand
        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]
        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg],2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]
        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        #find the XOR and format the string so that there will be leading zeros to match 16 bits
        result = format((int(registers["A"],2) ^ int(actualOperand,2)),"016b")
        registers["A"] = result

        SF=ZF=0
        if int(result,2) == 0:
            ZF=1
        if result[0:1] == "1":
            SF=1

    # find the AND of the value in register A and operand
    elif opcode == 9:   #AND

        actualOperand = ""
        if addressMode == "00":
            actualOperand = operand
        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]
        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]
        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        #find the result and format it to match 16 bits
        result = format((int(registers["A"],2) & int(actualOperand,2)), "016b")
        registers["A"] = result

        SF=ZF=0
        if int(result,2) == 0:
            ZF=1
        if result[0:1] == "1":
            SF=1

    # find the OR of the value in register A and operand
    elif opcode == 10:  #OR

        actualOperand = ""
        if addressMode == "00":
            actualOperand = operand
        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]
        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]
        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        # find the result and format it to match 16 bits
        result = format((int(registers["A"], 2) | int(actualOperand, 2)), "016b")
        registers["A"] = result

        SF=ZF=0
        if int(result, 2) == 0:
            ZF = 1
        if result[0:1] == "1":
            SF = 1

    #take the not of operand
    elif opcode == 11:  #NOT

        actualOperand = ""
        if addressMode == "00":
            actualOperand = operand
        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]
        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]
        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        #use the function defined at the beginning of the program
        result = takeNot(actualOperand)

        registers["A"]=result

        ZF = SF = 0
        if int(result, 2) == 0:
            ZF = 1
        if result[0:1] == "1":
            SF = 1

    #shift the bits of operand 1 unit to the left
    elif opcode == 12:  #SHL

        actualOperand = ""
        if addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        #find the result and format it to match 16 bits
        result = format( (int(actualOperand,2) << 1),"016b")

        CF = SF = ZF = 0
        if len(result) == 17:
            CF=1
            result = result[1:]
        if result[0:1] =="1":
            SF = 1
        if int(result,2) == 0:
            ZF=1

        registers[reg] = result

    # shift the bits of operand 1 unit to the right
    elif opcode == 13:  #SHR

        actualOperand = ""
        if addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        # find the result and format it to match 16 bits
        result = format((int(actualOperand, 2) >> 1), "016b")

        SF = ZF = 0     #Beginning of binary string is always 0 so sign flag is always 0
        if int(result, 2) == 0:
            ZF = 1

        registers[reg] = result

    #do nothing just update PC
    elif opcode == 14:  #NOP
        registers["PC"] = registers["PC"] + 3
        continue

    #push the operand into stack
    elif opcode == 15:  #PUSH

        if addressMode == "01":
            reg = registerEncodings[operand]
            #since stack is starting from high and goes to low put values accordingly
            memory[registers["S"]] = registers[reg][0:8]
            memory[registers["S"]-1] = registers[reg][8:]
            registers["S"] = registers["S"] - 2        #update S value by subtracting 2

    #pop from the stack
    elif opcode == 16:  #POP

        #if stack is empty POP operation causes an error
       if registers["S"] == 2**16-1 :
           fileOut.close()
           fileOut = open(sys.argv[1][:-3] + "txt", "w")
           fileOut.write("Error, pop from an empty stack")
           break


       if addressMode == "01":
           reg = registerEncodings[operand]
           registers[reg] = memory[registers["S"]] + memory[registers["S"] + 1]
           #update popped memory addresses to binary zero
           memory[registers["S"]] = "00000000"
           memory[registers["S"] + 1] = "00000000"
           registers["S"] = registers["S"] + 2          #update S value by adding 2

    #subtract operand from register A and set flags
    elif opcode == 17:  #CMP

        actualOperand = ""
        if addressMode == "00":
            actualOperand = operand

        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg], 2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]

        negatedOperand = takeComplement(actualOperand)
        result = addBin(registers["A"], negatedOperand)

        CF = SF = ZF = 0

        # if int(actualOperand,2) == 0:
        #    CF = 1

        if len(result) == 17:
            CF = 1
            result = result[1:]
        if result[0] == "1":
            SF = 1
        if int(result, 2) == 0:
            ZF = 1

    #jump the given address
    elif opcode == 18:  #JMP
        if addressMode == "00":
            registers["PC"] = int(operand, 2)
            continue

    elif opcode == 19:  #JZ JZE
        if addressMode == "00":
            if ZF == 1:
                registers["PC"] = int(operand, 2)
                continue

    elif opcode == 20:  #JNZ JNE
        if addressMode == "00":
            if ZF == 0:
                registers["PC"] = int(operand, 2)
                continue

    elif opcode == 21:  #JC
        if addressMode == "00":
            if CF == 1:
                registers["PC"] = int(operand, 2)
                continue

    elif opcode == 22:  #JNC
        if addressMode == "00":
            if CF == 0:
                registers["PC"] = int(operand, 2)
                continue

    #jump if A-X>0
    elif opcode == 23:  #JA

        if addressMode == "00":
            if SF == 0 and ZF == 0:
                registers["PC"] = int(operand,2)
                continue

    #jump if A-X>=0
    elif opcode == 24:  #JAE

        if addressMode == "00":
            if SF == 0 or ZF == 1:
                registers["PC"] = int(operand, 2)
                continue

    #jump if A-X<0
    elif opcode == 25:  #JB

        if addressMode == "00":
            if SF == 1 and ZF == 0:
                registers["PC"] = int(operand, 2)
                continue

    #jump if A-X<=0
    elif opcode == 26:  #JBE

        if addressMode == "00":
            if SF == 1 or ZF == 1:
                registers["PC"] = int(operand, 2)
                continue

    #read from user and store it
    elif opcode == 27:  #READ

        temp = input()                          #prompt user to give an input
        temp = format(ord(temp),"016b")         #turn character into ascii representation and match 16 bits

        if addressMode == "01":
            reg = registerEncodings[operand]
            registers[reg] = temp

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg],2)
            memory[memoryAddress] = temp[0:8]
            memory[memoryAddress + 1] = temp[8:]

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            memory[memoryAddress] = temp[0:8]
            memory[memoryAddress + 1] = temp[8:]

    #print the operand
    elif opcode == 28:  #PRINT

        # turn the binary string to character first

        if addressMode == "00":
            fileOut.write(chr(int(operand,2)))

        elif addressMode == "01":
            reg = registerEncodings[operand]
            actualOperand = registers[reg]
            fileOut.write( chr(int(actualOperand,2)) )

        elif addressMode == "10":
            reg = registerEncodings[operand]
            memoryAddress = int(registers[reg],2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress+1]
            fileOut.write(chr(int(actualOperand,2)))

        elif addressMode == "11":
            memoryAddress = int(operand,2)
            actualOperand = memory[memoryAddress] + memory[memoryAddress + 1]
            fileOut.write( chr( int(actualOperand,2) ))

        fileOut.write("\n")

    #move on to the next instruction
    registers["PC"]=registers["PC"]+3



