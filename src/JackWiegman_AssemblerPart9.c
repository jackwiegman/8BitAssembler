/**
 * @file SourceFiles/JackWiegman_AssemblerPart9.c
 * @brief  8 bit assembler and virtual machine that converts an ASM file to machine code and runs
 * the machine code.
 *
 * The program has two main components, the assembler portion, and virtual machine portion.
 * The Assembler portion of the program converts a given ASM file to machine code and places the
 * commands into an array of virtual 8 bit memory locations.
 * The Virtual Machine takes this array of memory, and executes the instructions from the machine
 * code that the Assembler converted.
 * After the running of the converted program, the final address space will be printed showing the
 * result of the execution.
 *
 * @version Assembler Part 9
 *
 * @author Jack Wiegman, S02497243
 * @date 2026-05-05
 *
 *(i/o files):
 * @include  ASMFiles/JackWiegman_ASMPart9.asm
 */

#define _CRT_SECURE_NO_WARNINGS  // lets us use deprecated code

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Default filename to run if user picks default. User can still type in other .asm files to run
char ASM_FILE_NAME[] = "JackWiegman_ASMPart9.asm";

#define MAX 150       // strlen of simulators memory can be changed
#define COL 9         // number of columns for output
#define LINE_SIZE 20  // For c-strings

// OPERAND TYPES, REGISTERS AND OTHER
#define AXREG 0     // 000 00 000
#define BXREG 1     // 000 00 001
#define CXREG 2     // 000 00 010
#define DXREG 3     // 000 00 011
#define BXADDR 4    // 000 00 100
#define BXPLUS 5    // 000 00 101
#define ADDRESS 6   // 000 00 110
#define CONSTANT 7  // 000 00 111

// Commands
#define RET 3
#define FUN 4
#define HALT 5
#define GET 6
#define PUT 7
#define JE 8
#define JNE 9
#define JB 10
#define JBE 11
#define JA 12
#define JAE 13
#define JMP 14
#define CMP 96
#define SUB 128
#define ADD 160
#define MOVREG 192
#define MOVMEM 224  // 111 00 000

// boolean
#define TRUE 1
#define FALSE 0

enum operType
{
    reg,
    mem,
    constant,
    arrayBx,
    arrayBxPlus,
    none
};  // list of all types of operand types

// Registers and flag add the stack pointer
struct Registers
{
    int AX;
    int BX;
    int CX;
    int DX;
    int flag;
} regis;

// GLOBAL VARIABLES
typedef short int Memory;    // sets the type of memory to short int
Memory memory[MAX] = { 0 };  // global variable the memory of the virtual machine
Memory address;              // global variable the current address in the virtual machine
Memory stackPointer;         // global variable with the current address of the stack

/**********  Assembler execution and functions  **********/
void assembler();                     // Converts the entire ASM file and stores it in memory
void getFileName( char filename[] );  // assembler helper to get filename of asm file to open.

void convertToMachineCode( FILE* fin );  // Converts a single line of ASM to machine code
                                         // from line at lineIndex and puts it in part
void convert3PartCommandToMC(
    Memory command, char part2[], char part3[] );  // Converts a 3 part ASM command to machine code.
Memory getJumpCommand(
    char commandString[LINE_SIZE] );  // Get machine code value for specified jump command

// Conversion helper functions
void splitCommand( char line[], char part1[], char part2[],
    char part3[] );  // splits line of asm into it's three parts
int getPartFromCommand(
    char line[], char part[], int lineIndex );  // Helper for splitCommand(). gets a single part

/**********  VM Execution and functions  **********/
void runMachineCode();  // Executes the machine code

void putValue( Memory operand, int value );  // Puts value given into specified operand
int getValue( int operand );                 // Returns the value of specified operand

// VM Stack operations
void push( int value );  // Push value onto stack
int pop();               // Pop value off stack

void runPUTCommand();                                    // Executes PUT command
void runGETCommand();                                    // Execute get command to put input into AX
void runADDCommand( Memory operand1, Memory operand2 );  // Executes machine code ADD command
void runSUBCommand( Memory op1, Memory op2 );            // Executes machine code SUB command.
void runMOVREGCommand( Memory operand1, Memory operand2 );  // Executes machine code MOVREG command
void runMOVMEMCommand( Memory part2MC, Memory part3MC );    // Executes machine code MOVMEM command
void runCMPCommand(
    Memory operand1, Memory operand2 );  // Executes machine code CMP command and sets FLAG
void runJumpCommands(
    Memory jumpType, int jumpLocation );  // Executes jump command based on FLAG state
// Command helpers
void jumpToLocation( int memLocation );  // Moves current address to new mem location

/**********  General Helper Functions  **********/
int changeToNumber( char line[],
    int* start );  // converts a sub-string to an int, may change function so start is
                   // passed by reference so it is the location directly after the number

int whichOperand( char operand[] );     // Returns the number of the register or other operand
void changeToLowerCase( char line[] );  // Changes each character in the string to lower case
void printMemoryDump();                 // Prints memory with commands represented as integers
void printMemoryDumpHex();              // Prints memory in hexadecimal
int isDigitOrNeg( char letter );        // is a character the start of a positive or negative number
void registerStartValues();             // gives all registers & flag random values to start

int main()
{
    assembler();
    printf( "\n\nConvert to Memory complete\n" );
    printMemoryDump();  // may also have printMemoryDumpHex but you must have printMemoryDump

    runMachineCode();
    printf( "\n\nFinal Memory dump in integers\n" );
    printMemoryDump();  // displays memory with final values
    printf( "\n\nFinal Memory dump in hex\n" );
    printMemoryDumpHex();  // displays memory with final values in hex

    printf( "\n" );  // puts a blank line betwwen the code output and the system output
    return 0;
}

/********************   assembler   ***********************/
/**
 * @brief Changes the assembly code to machine code and places the commands into memory.
 *
 * Prompts user for filename of ASM code to execute. Uses default if 'd' entered.
 */
void assembler()
{
    registerStartValues();  // Initialize Starting registers.
    address = 0;

    FILE* fin;  // File pointer for reading in the assembly code.

    /***** File Handling *****/
    char userAsmFilename[MAX] = "";  // Prompt and scan for filname
    getFileName( userAsmFilename );
    printf( "Filename: %s\n", userAsmFilename );

    // fin = fopen( ASM_FILE_NAME, "r" );  // Try to open file

    fin = fopen( userAsmFilename, "r" );  // Try to open file

    if ( fin == NULL )  // Exit program if no file exists or invalid input
    {
        printf( "Error, file didn't open\n\nExiting program...\n\n" );
        system( "pause" );
        exit( 1 );
    }

    /***** Start Conversion *****/
    for ( int i = 0; i < MAX && !( feof( fin ) ); i++ )
    {
        convertToMachineCode( fin );
        // printMemoryDumpHex();
        // printMemoryDump(); // debugging.
    }
}

/**
 * @brief Get's filename from user input.
 *
 * Puts user inputted filename into given filename parameter. Puts default filename if user chooses
 * so.
 *
 * @param filename[out] array to hold the users filename.
 */
void getFileName( char filename[] )
{
    printf( "Enter ASM filename to open ('d' for default): " );
    scanf( "%s", filename );

    if ( strcmp( filename, "d" ) == 0 )
    {  // Check for default name and set it if user chooses
        printf( "Default Selected.\n" );

        int count = 0;
        // Need to loop and add each char individually since sizes of arrays may be different
        while ( ASM_FILE_NAME[count] != '\0' )
        {  // Loop until null byte
            filename[count] = ASM_FILE_NAME[count];
            count++;
        }
        filename[count] = '\0';
    }
    // printf( "Filename: %s\n", filename );
}

/********************   convertToMachineCode   ***********************/
/**
 * @brief Converts a line of ASM code to 8 bit machine code.
 *
 * Read the next line from the file object, storing the line in an array and initializing arrays for
 * each part of the line.
 * Check's which command is being run, then get's the next operands if needed.
 * Converts each part of the command to the proper machine code and shifts so that the memory
 * address holds 8 bits in the form xxx yy zzz, where x is the first part of the command, y is the
 * second part, and z is the third/final.
 * For operands with CONSTANT/ADDRESS, puts the value for CONSTANT/ADDRESS in command and the
 * numeric literal in next memory location.
 *
 * @param fin Input file object to read ASM code from.
 */
void convertToMachineCode( FILE* fin )
{
    char line[MAX];              // full command
    char part1[LINE_SIZE];       // the asm commmand
    char part2[LINE_SIZE] = "";  // the two operands, could be empty
    char part3[LINE_SIZE] = "";
    int machineCode = 0;  // One line of converted asm code from the file

    if ( fgets( line, MAX, fin ) == NULL )
    {  // Takes one line from the asm file. Returns on error.
        // printf( "ERROR: line is null...\n" );
        return;
    }
    if ( line[0] == ';' )
    {  // if line is a comment (semi-colon in first column), skip location and don't increment
       // address
        return;
    }

    if ( line[0] == '\n' || line[0] == '\0' )
    {  // Increment address and go to next line if blank.
        address++;
        return;
    }

    int charCount = 0;
    if ( isDigitOrNeg( line[0] ) )
    {  // If start of line is a number, put the full number in memory then continue
        int numLiteral = changeToNumber( line, &charCount );
        memory[address] = numLiteral;
        address++;
        return;
    }
    changeToLowerCase( line );

    /******  Functions  ********/
    if ( line[0] == 'f' )
    {  // FUN command
        // fun {addr_of_func} {num_parameters} {param1} {param2}...{list each param}
        // {blankline/return_value}

        memory[address] = FUN;  // Put FUN command in mem
        address++;

        int lineIndex = 3;  // Char after fun
        // Get fuction address after next space
        int funcAddr = changeToNumber( line, &lineIndex );
        memory[address] = funcAddr;
        address++;

        // Get num parameters
        int numParams = changeToNumber( line, &lineIndex );

        memory[address] = numParams;
        address++;

        for ( int i = 0; i < numParams; i++ )
        {  // For each parameter, skip next space, then put param in next address
            int param = changeToNumber( line, &lineIndex );
            memory[address] = param;
            address++;
        }
        address++;  // skip one more address for return value
        return;     // done with this line, return to get next line
    }  // end FUN

    splitCommand( line, part1, part2, part3 );  // Split line, FUN doesn't need split line

    // for debugging, comment out when you have most commands converted.
    // printf( "\nCommand = P1=%s P2=%s P3=%s", part1, part2, part3 );
    /******  1 part commands   ********/
    if ( part1[0] == 'h' )
    {  // HALT
        memory[address] = HALT;
        address++;
    }  // end HALT
    else if ( part1[0] == 'p' )
    {  // PUT
        memory[address] = PUT;
        address++;
    }  // end PUT
    else if ( part1[0] == 'g' )
    {  // GET
        memory[address] = GET;
        address++;
    }  // end GET
    else if ( part1[0] == 'r' )
    {
        memory[address] = RET;
        address++;
    }

    /******  2 part commands   ********/
    else if ( part1[0] == 'j' )
    {  // JMP commands

        Memory jmpMC = getJumpCommand( part1 );
        if ( jmpMC < 0 )
        {
            printf( "ERROR: Jump command couldn't be determined...\n" );
            return;
        }
        memory[address] = jmpMC;
        address++;
        int index = 0;
        memory[address] = changeToNumber( part2, &index );
        address++;  // Address at next unused location.
    }  // end JMP commands
    /******  3 part commands   ********/
    else
    {
        if ( part1[0] == 'm' )
        {  // MOVMEM/MOVREG
            Memory part2OperandMC = whichOperand( part2 );
            Memory part3OperandMC = whichOperand( part3 );

            if ( part2OperandMC > DXREG )
            {  // MOVMEM - If part 2 not a register.

                if ( part3OperandMC > DXREG )
                {  // Make sure only one operand is 3 bits
                    printf( "ERROR: Attempted to convert when both operands are > 3" );
                    return;
                }
                convert3PartCommandToMC( MOVMEM, part3, part2 );
            }
            else if ( part2OperandMC >= AXREG && part2OperandMC <= DXREG )
            {  // MOVREG - 2nd operand is register
                convert3PartCommandToMC( MOVREG, part2, part3 );
            }
            else
            {  // 2nd operand not register/constant/address
                printf( "ERROR: Unknown 2nd operand...\n" );
            }
        }
        else if ( part1[0] == 'a' )
        { /*****  ADD COMMAND  *****/
            convert3PartCommandToMC( ADD, part2, part3 );
        }
        else if ( part1[0] == 's' )
        {
            // SUB command
            convert3PartCommandToMC( SUB, part2, part3 );
        }
        else if ( part1[0] == 'c' )
        {  // CMP command
            convert3PartCommandToMC( CMP, part2, part3 );
        }
        else
        {  // Shouldn't come here if all commands are valid.
            printf( "\n\n========\nERROR ALERT\n======================" );
        }
        // convert3PartCommandToMC increments address to next empty location, so address is primed
        // for next loop

    }  // end of three part commands

}  // end convertToMachineCode

/**************    convertToMachineCode HELPER FUNCTIONS    **************/
/**
 * @brief Converts a 3 part ASM command to machine code and puts in in the next location in memory.
 *
 * Takes command in order of machine code operands. Puts command into memory, then converts operands
 * to machine code and places them in memory as well.
 * If 3rd operand is an ADDRESS or CONSTANT, put literal into next memory location.
 * Increments address so it points to next unused memory location when finished.
 *
 *
 * @param command Machine code value of the command.    xxx 00 000
 * @param part2 Part 2 of the 3 part command.           000 xx 000
 * @param part3 Part 3 of the 3 part command.           000 00 xxx
 */
void convert3PartCommandToMC( Memory command, char part2[], char part3[] )
{
    memory[address] += command;

    Memory part2Operand = whichOperand( part2 );
    memory[address] += ( part2Operand << 3 );  // 000 00 0xx -> 000 xx 000
    Memory part3Operand = whichOperand( part3 );
    memory[address] += part3Operand;

    address++;  // Done with mem location, increment address.

    int index = 0;
    if ( part3Operand > DXREG && part3Operand != BXADDR )
    {  // If CONST/ADDR/BXPLUS in part3, put literal in next mem location
        int part3Literal = changeToNumber( part3, &index );
        memory[address] = part3Literal;

        address++;  // Done with mem location, increment address.
    }
}

/**
 * @brief Get's machine code for specified jump command.
 *
 * @param commandString Sring holding just jump part of command.
 *
 * @return Machine code value of jump to run.
 */
Memory getJumpCommand( char commandString[LINE_SIZE] )
{
    switch ( commandString[1] )
    {  // Check 2nd letter for jump type

    case ( 'a' ): {  // JA/JAE
        switch ( commandString[2] )
        {  // Check 3rd letter for jump above type

        case ( 'e' ): {  // JAE
            return JAE;
        }
        case ( '\0' ): {  // JA
            return JA;
        }
        default: {  // Jump unknown
            printf( "ERROR: Jump above type unknown...\n" );
            return -1;
        }
        }
    }
    case ( 'b' ): {  // JB/JBE
        switch ( commandString[2] )
        {  // Check 3rd letter for jump below type

        case ( 'e' ): {  // JBE
            return JBE;
        }
        case ( '\0' ): {  // JB
            return JB;
        }
        default: {  // Jump unknown
            printf( "ERROR: Jump below type unknown...\n" );
            return -1;
        }
        }
    }
    case ( 'e' ): {  // JE
        return JE;
    }
    case ( 'n' ): {  // JNE
        return JNE;
    }
    case ( 'm' ): {  // JMP
        return JMP;
    }
    default: {  // Jump unknown
        printf( "ERROR: Jump type unknown...\n" );
        return -1;
    }
    }
}

/********************   splitCommand   ***********************/
/**
 * @brief Splits a line of ASM into it's parts
 *
 *
 * Initializes parts with null byte in case they don't get filled. Then parses one part of the
 * command at a time, putting them into their respective arrays. The lineIndex is updated with the
 * result of the last function call to keep track of the line of ASM.
 *
 * @param line Full line of ASM code to be split.
 * @param part1 Array to hold part1 of the command
 * @param part2 Array to hold part2 of the command
 * @param part3 Array to hold part3 of the command
 */
void splitCommand( char line[], char part1[], char part2[], char part3[] )
{
    int lineIndex = 0;  // the character location in the string line
    int partIndex = 0;  // character location in new string ie the parts
    part2[0] = '\0';    // initialize the strings part2 & 3 to empty strings
    part3[0] = '\0';    // in case there are only one or no operand.
    part1[0] = '\0';    // Initialize part 1 as well, so everything passed to function is the same

    lineIndex = getPartFromCommand( line, part1, lineIndex );
    lineIndex = getPartFromCommand( line, part2, lineIndex );
    lineIndex = getPartFromCommand( line, part3, lineIndex );

}  // end splitCommand
/*********************   getPartFromCommand   ***************************/
/**
 * @brief Get's the next part of the command and puts it into the command part.
 *
 * Helper function for splitCommand.
 * Gets the next word/command from the line and the given index, then puts the next command into
 * part and returns the new lineIndex.
 * Checks for spaces and moves forward until different character is reached.
 * Then loops through line starting at the index until the end of the command is reached.
 * The command part is update with the command and null char.
 * Function returns new lineIndex
 *
 * @param line Full line of ASM code being parsed.
 * @param part Array to put the next command into.
 * @param lineIndex Index of where to start parsing for the next command.
 * @return New index of the line after command is read.
 */
int getPartFromCommand( char line[], char part[], int lineIndex )
{
    int partIndex = 0;  // Initialize part index to 0 or first location in array

    while ( line[lineIndex] == ' ' )
    {  // If current character is a space, move until it's not.
        lineIndex++;
    }

    if ( line[lineIndex] != '\0' )
    {  // Check that end of string not reached
        while ( line[lineIndex] != ' ' && line[lineIndex] != '\0' && line[lineIndex] != '\n' )
        {  // Loop through line until end of command, or line.

            part[partIndex] = line[lineIndex];  // Move command from line to part array.
            partIndex++;                        // Increment indices
            lineIndex++;
        }
        part[partIndex] = '\0';  // Add string termination char.
    }
    return lineIndex;  // return new line index to keep track of where we are in array.
}  // end getPartFromCommand

/**************    END convertToMachineCode HELPER FUNCTIONS    **************/
/********************   END assembler   ***********************/

/******************** Virtual Machine Execution ***************/

/**
 * @brief Push value to stack.
 *
 * Push's value to current SP, then increments SP to next unused location.
 *
 * @param value Value to push.
 */
void push( int value )
{
    memory[stackPointer] = value;
    stackPointer--;
}

/**
 * @brief Peop top of stack.
 *
 * Increments SP up, then returns value. Since
 *
 * @return Value popped.
 */
int pop()
{
    if ( stackPointer < ( MAX - 1 ) )
    {  // Make sure stack doesn't go past memory size.
        stackPointer++;
        return memory[stackPointer];
    }
    else
    {
        printf( "Error: Stack Pointer attempting to overflow memory." );
        return -1;
    }
}

/********************   runMachineCode   ***********************/
/**
 * @brief Executes the machine code that is in the virtual machine memory.
 *
 * Resets address to first memory location of the virtual machine.
 * Loops until it hits 'halt' in the file. Splits the machine code into 3 parts, with part1 being
 * the command, and the other 2 parts as operands.
 * Function checks which ASM command is given in part one. And runs the command accordingly, using
 * operands and incrementing addresses as needed.
 * Once the machine code value of 'halt' appears, the loop exits and the program terminates after
 * printing the memory.
 */
void runMachineCode()
{

#define MASK1 224  // 111 00 000
#define MASK2 24   // 000 11 000
#define MASK3 7    // 000 00 111

    address = 0;                 // reset address to the first item in memory
    stackPointer = ( MAX - 1 );  // Set stack pointer to last mem location

    Memory fullCommand;  // Initialize full command before using in while loop, not yet broken into
                         // the three parts
    fullCommand = memory[address];  // Get command in memory location.

    while ( fullCommand != HALT )  // loop until halt
    {

        address++;  // Move to next address after current command. Address resides at next memory
                    // location that information should be taken from.

        /**********  1 PART COMMANDS  **********/
        if ( fullCommand == PUT )
        {  // PUT
            runPUTCommand();
            fullCommand = memory[address];
            continue;
        }
        else if ( fullCommand == GET )
        {  // GET
            runGETCommand();
            fullCommand = memory[address];
            continue;
        }
        else if ( fullCommand == FUN )
        {
            int funcAddr = memory[address];
            address++;
            int paramAddress = address;
            memory[funcAddr - 1] = paramAddress;  // Put paramNum in line before function.

            address++;  // Address now on first parameter;

            int paramNum = memory[paramAddress];  // Number of parameters for function

            for ( int i = 0; i < paramNum; i++ )
            {  // Put address on return location
                address++;
            }

            push( address );  // Push return address to stack.
            // Push registers to stack
            push( getValue( AXREG ) );
            push( getValue( BXREG ) );
            push( getValue( CXREG ) );
            push( getValue( DXREG ) );
            push( regis.flag );

            jumpToLocation( funcAddr );     // Jump to function body.
            fullCommand = memory[address];  // Set command
            continue;                       // restart loop
        }
        else if ( fullCommand == RET )
        {
            int retVal = getValue( AXREG );  // Get return value in AX.
            // Pop stack, reset registers to state before function.
            regis.flag = pop();
            regis.DX = pop();
            regis.CX = pop();
            regis.BX = pop();
            regis.AX = pop();

            // Pop last item: return address. Then set return address and jump to next location.
            int retAddress = pop();
            memory[retAddress] = retVal;
            jumpToLocation( retAddress + 1 );  // Jump to mem after return address.

            // Set command and restart loop
            fullCommand = memory[address];
            continue;
        }

        /**********  2 PART COMMANDS ***********/
        if ( fullCommand >= JE && fullCommand <= JMP )
        {                                        // If any jump command:
            int jumpLocation = memory[address];  // Get literal mem location.
            address++;                           // Address stays on next unused location.

            if ( fullCommand == JMP )
            {  // Jump no matter what
                jumpToLocation( jumpLocation );
            }
            else
            {
                runJumpCommands( fullCommand, jumpLocation );
            }

            fullCommand = memory[address];
            continue;  // Go to next loop iteration after jump or non-jump.
        }

        /**********  3 PART COMMANDS ***********/

        // Get parts of command in machine code.
        Memory part1MC = ( fullCommand & MASK1 );       // xxx 00 000
        Memory part2MC = ( fullCommand & MASK2 ) >> 3;  // 000 xx 000
        Memory part3MC = ( fullCommand & MASK3 );       // 000 00 xxx

        // Debugging: Print machine code of each part
        // printf( "part1=%d, part2=%d, part3=%d", part1MC, part2MC, part3MC );

        /********** 3 part commands **********/
        if ( part1MC == MOVREG )
        {  // MOVREG command
            runMOVREGCommand( part2MC, part3MC );
        }
        else if ( part1MC == ADD )
        {  // ADD command
            runADDCommand( part2MC, part3MC );
        }
        else if ( part1MC == SUB )
        {
            runSUBCommand( part2MC, part3MC );
        }
        else if ( part1MC == MOVMEM )
        {  // MOVMEM COMMAND
            runMOVMEMCommand( part2MC, part3MC );
        }
        else if ( part1MC == CMP )
        {  // CMP Command
            runCMPCommand( part2MC, part3MC );
        }
        else
        {  // Command unknown
            printf( "\nERROR: No matching operand for command: %d\n", part1MC );
            printf( "At address: %d", address );
            return;  // Exit program for error
        }

        // debugging tool change address to show what you are unsure about
        // if ( address > 0 )
        // {
        //     printMemoryDump(); // debugging, comment out at end of semester
        // }
        fullCommand = memory[address];  // Get command in memory location.

    }  // End while not HALT

}  // end runMachineCode

/**************    runMachineCode HELPER FUNCTIONS    **************/
/**********     ASM COMMAND FUNCTIONS     **********/
/**
 * @brief Executes the put command by printing register AX.
 */
void runPUTCommand()
{
    // printf( "\nPut register AX: %d  BX: %d  CX: %d  DX: %d\n", getValue( AXREG ), getValue( BXREG
    // ), getValue( CXREG ), getValue( DXREG ) );
    printf( "\nPut register AX: %d\n", regis.AX );
}

/**
 * @brief Executes GET command during run.
 *
 * Get's user input and puts value into AX register.
 */
void runGETCommand()
{
    printf( "Get (Enter input, then enter): " );
    int input = 0;
    scanf( "%d", &input );
    putValue( AXREG, input );
}

/**
 * @brief Executes move register command from machine code memory.
 *
 * @param operand1 Second part of ASM command, value to hold the sum.
 * @param operand2 Third part of ASM command, value to add to operand1.
 */
void runMOVREGCommand( Memory operand1, Memory operand2 )
{
    int value = getValue( operand2 );
    putValue( operand1, value );  // Put value into operand
}  // end runMOVREGCommand

/**
 * @brief Executes add register from machine code memory.
 *
 * @param operand1 Second part of ASM command, value to hold the sum.
 * @param operand2 Third part of ASM command, value to add to operand1.
 */
void runADDCommand( Memory operand1, Memory operand2 )
{
    int sum = getValue( operand1 ) + getValue( operand2 );
    putValue( operand1, sum );  // put the sum of operand 1 and 2 into operand 1
}  // end runADDCommand

/**
 * @brief Executes subtract command and places result in second operand.
 *
 * @param op1 Second part of asm command, will contain difference of op1 - op2.
 * @param op2 Third part of asm command, value to subtract.
 */
void runSUBCommand( Memory op1, Memory op2 )
{
    int difference = getValue( op1 ) - getValue( op2 );
    putValue( op1, difference );
}
/**
 * @brief Moves values into memory locations.
 *
 * Moves the value from the register into the specified location in memory.
 *
 * @example Command: `mov [ADDRESS] REGISTER`
 *      - function takes REGISTER as operand1 and [ADDRESS] as operand2.
 *
 * @param operand1 Part 2 of the command, second operand with REGISTER.
 * @param operand2 Part 3 of the command, first operand with ADDRESS.
 */
void runMOVMEMCommand( Memory operand1, Memory operand2 )
{
    if ( operand2 != ADDRESS && operand2 != BXADDR && operand2 != BXPLUS )
    {  // Extra check to make sure we're running the right command.
        printf( "ERROR: MOVMEM command without correct ADDRESS operand...\n" );
        return;
    }
    // Get reg value and address literal, then put value into address.
    int regValue = getValue( operand1 );
    putValue( operand2, regValue );
}  // end runMOVMEMCommand

/**
 * @brief Run CMP command in machine code.
 *
 * Compare two values. Set's flag based on the result of the comparison.
 *
 * Sets flag:
 *  - -1:   op1 < op2
 *  -  0:   op1 == op2
 *  -  1:   op1 > op2
 *
 * @param operand1 First operand of comparison. Must be a REGISTER AX,BX,CX,DX.
 * @param operand2 Second operand of comparison. May be REGISTER/CONSTANT/ADDRESS.
 */
void runCMPCommand( Memory operand1, Memory operand2 )
{
    // Get values
    int op1Value = getValue( operand1 );
    int op2Value = getValue( operand2 );

    // Compare values and set flag.
    if ( op1Value < op2Value )
    {  // op1 < op2 -> -1
        regis.flag = -1;
    }
    else if ( op1Value > op2Value )
    {  // op1 > op2 -> 1
        regis.flag = 1;
    }
    else
    {  // op1 == op2 -> 0
        regis.flag = 0;
    }
}  // end runCMPCommand

/**
 * @brief Executes specific jump command to location if flag is set to do so.
 *
 * Checks current flag value, then checks if the jump type should jump based on that flag. Sets
 * address to jumpLocation if jump should be executed, otherwise exits and does nothing.
 *
 * @param jumpType Machine code value of specific jump type.
 * @param jumpLocation Location to jump to if jump is executed.
 */
void runJumpCommands( Memory jumpType, int jumpLocation )
{
    // Check the flag, then check for jump commands that will jump on that flag.
    // Continue to run next loop iteration after jump.
    switch ( regis.flag )
    {
    case ( 1 ): {  // Value is greater than. Jump if JAE,JA,JNE.
        if ( jumpType == JAE || jumpType == JA || jumpType == JNE )
        {
            jumpToLocation( jumpLocation );
        }
        break;
    }
    case ( -1 ): {  // Values less than. Jump if JBE,JB,JNE.
        if ( jumpType == JBE || jumpType == JB || jumpType == JNE )
        {
            jumpToLocation( jumpLocation );
        }
        break;
    }
    case ( 0 ): {  // Values equal. Jump if JE, JAE, or JBE.
        if ( jumpType == JE || jumpType == JAE || jumpType == JBE )
        {
            jumpToLocation( jumpLocation );
        }
        break;
    }
    default: {  // Flag set incorrectly
        printf( "ERROR: Flag value unknown...\n" );
        return;
    }
    }  // end switch(regis.flag)
}

/**
 * @brief Jumps to memory location.
 *
 * Sets address to desired memory location so execution continues at that address.
 *
 * @param memLocation Memory location to jump to.
 */
void jumpToLocation( int memLocation )
{
    address = memLocation;
}
/**********     END ASM COMMAND FUNCTIONS     **********/

/********************   putValue   ***********************/
/**
 * @brief Puts a given value into a register.
 *
 * @param operand Operand to hold the value.
 * @param value Value to be placed in operand.
 */
void putValue( Memory operand, int value )
{
    switch ( operand )
    {  // Switch on the operand
    case AXREG: {
        regis.AX = value;
        break;
    }
    case BXREG: {
        regis.BX = value;
        break;
    }
    case CXREG: {
        regis.CX = value;
        break;
    }
    case DXREG: {
        regis.DX = value;
        break;
    }
    case BXADDR: {
        memory[regis.BX] = value;
        break;
    }
    case BXPLUS: {
        int plusNum = memory[address];
        address++;
        memory[regis.BX + plusNum] = value;
        break;
    }
    case ADDRESS: {
        int memLocationLiteral = memory[address];
        address++;
        memory[memLocationLiteral] = value;
        break;
    }
    case CONSTANT: {
        printf( "Cannot store values in a constant." );
        break;
    }
    default: {
        printf( "putValue: Operand not valid..." );
        break;
    }
    }
}
/********************   getValue   ***********************/
/**
 * @brief Gets the value of the operand given.
 *
 * Assumes current address is pointing to next memory location after latest command.
 *
 * @param operand Operand to get value of.
 * @return Returns value held by operand. If operand is CONSTANT/ADDRESS, returns value of literal
 * in next address and increments address so it points to next mem location after literal.
 */
int getValue( int operand )
{
    switch ( operand )
    {
    case AXREG: {
        return regis.AX;
    }
    case BXREG: {
        return regis.BX;
    }
    case CXREG: {
        return regis.CX;
    }
    case DXREG: {
        return regis.DX;
    }
    case BXADDR: {
        return memory[regis.BX];
    }
    case BXPLUS: {
        int plusNum = memory[address];
        address++;
        return memory[regis.BX + plusNum];
    }
    case ADDRESS: {
        int addrValue = memory[address];
        address++;
        return memory[addrValue];
    }
    case CONSTANT: {
        int constValue =
            memory[address];  // value we want is in current memory address (after command)
        address++;            // increment to next address since we're done with this one now
        return constValue;
    }
    default: {
        printf( "No value could be retrieved..." );
        return 0;
    }
    }
    return 0;
}

/************** END runMachineCode HELPER FUNCTIONS    **************/

/*********************   whichOperand   ***************************/
/**
 * @brief Determines which operand is being referenced and returns machine code value.
 *
 * Operands:
 *      0-3 registers
 *      4 bx addrsss - added in part 7
 *		5 bx address plus offset -added in part 7
 *		6 address -added in part 5
 *		7 constant
 *
 * @param operand operand string to convert to machine code value.
 *
 * @return machine code value of the register/constant/address
 */
int whichOperand( char operand[LINE_SIZE] )
{
    int index = 0;
    char letter = operand[index];
    while ( letter == ' ' || letter == '\t' )
    {
        index++;
        letter = operand[index];
    }
    if ( letter == 'a' )
    {
        return AXREG;
    }
    else if ( letter == 'b' )
    {
        return BXREG;
    }
    else if ( letter == 'c' )
    {
        return CXREG;
    }
    else if ( letter == 'd' )
    {
        return DXREG;
    }
    else if ( letter == '[' )
    {
        if ( operand[1] == 'b' )
        {  // check for [BX] and [BX+]
            if ( operand[3] == '+' )
            {  // Check for '+'
                return BXPLUS;
            }
            return BXADDR;  // no plus, just BXADDR
        }
        return ADDRESS;  // no b, just normal address.
    }
    // ---------------
    else if ( isDigitOrNeg( letter ) )
    {
        return CONSTANT;
    }
    return -1;  // something went wrong if -1 is returned
}  // end whichOperand

/*********************ChangeToNumber ********************/
/**
 * @brief Takes in a line and converts digits to a integer.
 *
 * NOTE: may be changed so that the start is passed by reference.
 *
 * @param line string of assembly code to convert
 * @param start[in/out] location where line is being converted. Updated with index of char after
 * current num.
 *
 * @return integer value of the string
 */
int changeToNumber( char line[], int* start )
{
    int value;         // is the integer value of the digits in the code
    char number[16];   // just the digits
    int negative = 0;  // negative or positive number

    int i = 0;
    while ( line[*start] == '[' || line[*start] == ' ' || line[*start] == ']' )
    {
        ( *start )++;
    }
    if ( line[*start] == '-' )
    {
        ( *start )++;
        negative = 1;
    }
    else if ( line[*start] == 'b' )
    {  // Getting BXPLUS value
        if ( line[*start + 2] == '+' )
        {                 // If BXPLUS
            *start += 3;  // Put start at numeric value after BX+
        }
    }
    while ( i < 16 && isdigit( line[*start] ) )
    {
        number[i] = line[*start];
        i++;
        ( *start )++;
    }
    number[i] = '\0';
    value = atoi( number );
    if ( negative == 1 )
    {
        value = -value;
    }
    return value;
}  // end convertToNumber

/*********************   changeToLowerCase   ********************/
/**
 * @brief Changes each character in the line to lower case.
 *
 * @param line string to convert, line is completely changed to lower case.
 */
void changeToLowerCase( char line[] )
{
    int index = 0;
    while ( index < strlen( line ) )
    {
        line[index] = tolower( line[index] );
        index++;
    }
}  // end changeToLowerCase

/*********************   isDigitOrNeg   ********************/
/**
 * @brief Determines if a character is the start of a number positive or negative.
 *
 *      true (1): if the character is a digit or a negative sign
 *      false (0): otherwise
 *
 * @param letter a character
 *
 * @return if char is number or not.
 */
int isDigitOrNeg( char letter )
{
    int retValue = 0;
    if ( letter == '-' || isdigit( letter ) )
    {
        retValue = 1;
    }
    return retValue;
}  // end isDigitNeg

/*********************  registerStartValues  *****************************/
/**
 * @brief Gives all registers and the flag random values to start the code.
 *
 *  This is realistic because unless the machine is just been turned on the values will be unknown.
 *  The values are upto 4 digits. The formula for AX = (high - low + 1)+low;
 *      high = 9999
 *      low = 2
 */
void registerStartValues()
{
    srand( 0 );  // starts random number generator, will always have the same values, best for
                 // debugging
    // srand(time(NULL) ); //starts random generator with a the time, will appear more random
    regis.AX = rand() % ( 9999 - 2 + 1 ) + 2;
    regis.BX = rand() % 9998 + 2;
    regis.CX = rand() % 9998 + 2;
    regis.DX = rand() % 9998 + 2;
    regis.flag = rand() % 9998 + 2;
}  // end of registerStartValues

/*********************************************************************************/
/***********************   HELPER FUNCTIONS for debugging  ***********************/
/*********************************************************************************/

/****************************   printMemoryDump   ********************************/
/**
 * @brief Prints memory out as integers.
 *
 *  MAX (constant) is the amount of elements in the memory memory array
 *  COL (constant) is the number of columns that are to be displayed
 */
void printMemoryDump()
{
    int numRows = MAX / COL + 1;  // number of rows that will print
    int carryOver = MAX % COL;    // number of columns on the bottom row
    int location;                 // the current location being called
    printf( "\n" );
    for ( int row = 0; row < numRows; row++ )
    {
        location = row;
        for ( int column = 0; location < MAX && column < COL; column++ )
        {
            if ( !( numRows - 1 == row && carryOver - 1 < column ) )
            {
                printf( "%4d.%5d", location, memory[location] );
                location += ( numRows - ( carryOver - 1 < column ) );
            }
        }
        printf( "\n" );
    }
    printf( "\nAX: %d", regis.AX );
    printf( "%7s: %d", "BX", regis.BX );
    printf( "%7s: %d", "CX", regis.CX );
    printf( "%7s: %d", "DX", regis.DX );
    printf( "%9s: %d\n", "flag", regis.flag );
    printf( "Address: %d\t", address );
    printf( "Stack Pointer:%d\n\n", stackPointer );
}  // end printMemoryDump

/*********************   printMemoryDumpHex   ********************/
/**
 * @brief Prints memory in hexidecimal.
 *
 *  MAX is the amount of elements in the memory memory array
 *  COL is the number of columns that are to be displayed
 */
void printMemoryDumpHex()
{
    int numRows = MAX / COL + 1;  // number of rows that will print
    int carryOver = MAX % COL;    // number of columns on the bottom row
    int location;                 // the current location being called
    printf( "\n" );
    for ( int row = 0; row < numRows; row++ )
    {
        location = row;
        for ( int column = 0; location < MAX && column < COL; column++ )
        {
            if ( !( numRows - 1 == row && carryOver - 1 < column ) )
            {
                printf( "%4d.%4x", location, memory[location] );
                location += ( numRows - ( carryOver - 1 < column ) );
            }
        }
        printf( "\n" );
    }
    printf( "\nAX: %d", regis.AX );
    printf( "%7s: %d", "BX", regis.BX );
    printf( "%7s: %d", "CX", regis.CX );
    printf( "%7s: %d", "DX", regis.DX );
    printf( "%9s: %d\n", "flag", regis.flag );
    printf( "Address: %d\t", address );
    printf( "Stack Pointer:%d\n\n", stackPointer );
}  // end printMemoryDumpHex

/**
 */
