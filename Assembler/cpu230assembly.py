import sys
import re

#Dictionary of all opcodes as keys and their corresponding hexadecimals as values.
OpcodesAll = {
    "LOAD":"2", "ADD":"4", "SUB":"5", "INC":"6", "DEC":"7", "XOR":"8", "AND":"9", "OR":"A", "NOT":"B", "NOP":"E", "CMP":"11", "PRINT":"1C",
    "JMP":"12", "JZ":"13", "JE":"13", "JNZ":"14", "JNE":"14", "JC":"15", "JNC":"16", "JA":"17", "JAE":"18", "JB":"19", "JBE":"1A",
    "STORE":"3", "READ":"1B",
    "SHL":"C", "SHR":"D", "PUSH":"F", "POP":"10"
}

#converts 6 bit binary opcode, 2 bit binary addressing mode and 16 bit binary operand to 6 bit hex instruction.
#this function is copied from shared .py code by Prof. Can Özturan.
def convert(opcode, addrmode, operand):
    opcode = int(opcode,16) 
    addrmode = int(addrmode,16) 
    operand  = int(operand,16) 

    bopcode = format(opcode, '06b') 
    baddrmode = format(addrmode, '02b') 
    boperand = format(operand, '016b') 
    bin = '0b' + bopcode + baddrmode + boperand 
    ibin = int(bin[2:],2) ; 
    instr = format(ibin, '06x') 
    return instr+"\n"


input = open(sys.argv[1], "r")  #input file to read instructions,
data = open(sys.argv[1], "r")   #input file to store labels to dictionary.
outputName =sys. argv[1][0:-4]
out = open(outputName+".bin", "w")  #output file.

remember = {} 
location = 0 #memory location
for line in data:
    ws = re.fullmatch(r'^\s+$', line) #if line is just space, then continue.
    if(ws): continue

    line = line.strip()
    label = re.search('\w+:', line) #if line contains word characters followed by colon, then store it as label.
    if(label):
        line = line[:-1].upper()
        if(line in remember): #if label already defined,
            print("Error: Multiple definition of label: " + line) #produce error output.
            exit()
        remember.update({line:hex(location*3)}) #every instruction is 3 bytes.
    else:
        location += 1 #update the location.

for line in input:

    ws = re.fullmatch(r'^\s+$', line) #if line is just space, then continue.
    if(ws): continue
    
    line = line.strip()
    tokens = line.split() #seperate token by token.
    
    opcode = tokens[0] #first token is opcode.
    
    if(opcode.upper() == "HALT"): #HALT instruction only produces one output.
        out.write("040000\n")

    elif(opcode.upper() == "NOP"): #NOP instruction only produces one output.
        out.write("380000\n")
    
    elif(opcode.upper() in OpcodesAll): #check if opcode is in opcodes map.
        operand = tokens[1] #next token is operand. 
        
        #determine the addressing mode of the operand using regex.
        memory = re.fullmatch(r'^\[([0-9A-F]){4}\]$', operand)
        meminreg = re.fullmatch(r'^\[([A-E]|PC|S)\]$', operand)
        register = re.fullmatch(r'^([A-E]|PC|S)$', operand)
        immediate = re.fullmatch(r'^((\'.*\')|(\".*\")|((%d|A-F){4})|(\w+))$', operand)

        #if there is a 3rd token, then the addressing mode is immediate
        if(len(tokens) == 3):
            dindex = line.find("\"")
            sindex = line.find("\'")
            if(dindex != -1):
                operand = line[dindex:] #extract operand.
                register = False
                immediate = True
            if(sindex != -1):
                operand = line[sindex:] #extract operand.
                register = False
                immediate = True

        if(register):
            am = '1'    #set addressing mode to 01
            #change operand to registers value.
            if(operand == 'A'): operand = '1'
            if(operand == 'B'): operand = '2'
            if(operand == 'C'): operand = '3'
            if(operand == 'D'): operand = '4'
            if(operand == 'E'): operand = '5'
            if(operand == 'S'): operand = '6'
            if(operand == 'PC'): operand = '0'

        elif(immediate):
            am = '0'    #set addressing mode to 00
            if(operand[0] == '\''): #if directly given
                operand = hex(ord(operand[1:-1]))
            elif(operand[0] == '\"'): #if directly given
                operand = hex(ord(operand[1:-1]))
            elif(operand in remember): #if given as label
                operand = remember[operand]
            if(len(operand) > 5): #check operand
                print("Error: Invalid Operand:" + operand)
                exit()
            elif(len(operand) == 5):
                if(operand[0] != '0'):
                    print("Error: Invalid Operand:" + operand)
                    exit()

        elif(meminreg):
            am = '2'    #set addressing mode to 10
            #change operand to registers value.
            if(operand == '[A]'): operand = '1'
            if(operand == '[B]'): operand = '2'
            if(operand == '[C]'): operand = '3'
            if(operand == '[D]'): operand = '4'
            if(operand == '[E]'): operand = '5'
            if(operand == '[S]'): operand = '6'
            if(operand == '[PC]'): operand = '0'

        elif(memory):
            am = '3'    #set addressing mode to 11
            operand = operand[1:-1]
        
        instr = convert(OpcodesAll[opcode.upper()], am, operand) #calculate instruction.
        out.write(instr.upper()) #print instruction to output file.