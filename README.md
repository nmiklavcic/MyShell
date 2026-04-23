**Repo for creating a version of the shell**
 - homework assignment for my operating systems class
 - Need to create a working prototype of the shell in C
 - First part of the assignment is to create the REPL (read evaluate print loop):
 - REPL instructions : 
   - This is the root of the shell
   - We need the following functions 
     - tokenize -> turns the commands in the io stream in to seperate symbols
     - parse    -> character recognition
   - This must be done using only "low-level" functions such as:
     - "System calls"                 -> write(), fork(), exit()...
     - Memory allocation functions    -> malloc(), free(), ...
     - String manipulation functions  -> strlen(), strcmp(), memcpy()
     - Type conversion functions      -> atoi()
     - Basic read and print functions -> printf(), scanf()

