# bms-feb
A short demonstration of a BMS state machine for the Berkeley Formula Electric team. 
This code takes in a series of inputs at the beginning and effectively does a one-pass run through the state machine.

## Instructions

1. Clone the repository via _git clone <respository_name.git>_
2. Compile the program via _gcc -o main.c main_
3. A new file _main_ will be generated.
4. Run via ./main in your local terminal.
5. voila! you've got a running BMS state machine.

## If Compilation Errors Occur:

1. Try removing all instances of sleep()
2. Try setting a C version (say C99) when compiling via the command: _gcc -std=c99 -o main main.c_
