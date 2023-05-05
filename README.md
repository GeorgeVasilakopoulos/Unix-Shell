# Systems Programming Assignment 1

### Compilation: ```make``` 
### Execution: ```./mysh```.


## General 

- The execution of the shell program begins at ```src/interface.c```. Once the necessary structures are initialized, the program awaits for an instruction to be given. Once an instruction is given, it is passed as a string to the ```interpretInstruction()``` function, defined at ```src/interpreter.c```.

- The ```interpretInstruction()``` function works by splitting the string into 'tokens' and storing them in a list structure (```src/lexer.c```). Then, it proceeds to transform the token list so that aliases are replaced, quotations are removed etc. and then, it begins to intepret the instruction by examining the tokens in a serial order.

- **Anything contained within single ```'``` or double ```"``` quotes is considered a single token**. Therefore, environment variables, wild character sequences or aliases within quotes are treated as **simple strings**. For example, ```echo "*.txt"``` simply prints ```*.txt``` and ```createalias myalias "echo 123"``` followed by ```myalias```, will print ```echo 123: command not found```.


- The following meta-instructions can only be executed individually, meaning that they cannot be executed in series ```;```, or in parallel ```&;``` and their output cannot be redirected to files, or other instructions:

    - ```createalias```
    - ```destroyalias```
    - ```exit```
    - ```myHistory```
    - ```prev```

- Also, ```cd``` is treated as an exception and its output cannot be redirected into files.

## Background Execution

- If one of the end tokens (; or &;) is found during the serial evaluation, the instruction that preceeded is executed by forking the shell process (```src/interface.c```) and calling ```execvp```. The original (parent process) either waits for the child process to terminate (;) or proceeds to interpret the rest of the given instruction (&;).   

- If an instruction is executed in the background, the shell process does not supervise the execution in any way. 

## Input/Output Redirections

- When an instruction is interpreted, initially, the input and output file streams are set to the standard values of 0 and 1. If one of the following tokens is observed (```>```, ```<```, ```>>```), then the Input-Output file descriptors ```IOfd[]``` change appropriately, to correspond to the requested file streams.

- If one of the end tokens is observed, then the IO file descriptors are reset to 0 and 1.

## Piped Instructions

- When the token ```|``` is observed, a pipe is created internally and, if the Output file descriptor of the previous instruction was the console (1), then it is reset to pass the output through the pipe. Also, the Input file descriptor of the instruction that proceeds is set to the output of the pipe.

- By default, ```|``` implies parallel execution of the piped instructions. 

- If the Output fd of the instruction prior to ```|``` is not the console, then the input end of the created pipe is not assigned to any instruction.


## Signal Handling

- ```Ctr + C``` terminates the execution of the running instruction, by passing ```SIGKILL```. 

- If multiple instructions are given in series, using ```;```, then the execution will continue after the next semicolon.

- ```Ctr + Z``` is passed as is to the running instruction.

- Neither of these signals effects the running shell.

## Wild Characters

- The function ```replaceWildTokens()```, ```src/alias.c``` performs all the necessary transformations to the token list that is passed as argument.

- If a token in the list is considered 'wild', then the shell executes an ```ls``` command internally.

- The output of the ```ls``` command is split into tokens and, using functions of the ```regex``` library, it is determined whether the files of the directory match the given pattern. The tokens that match the pattern are put in place of the corresponding wild token. 

- If a token list contains more than one wild tokens, then the comparison process is performed for every wild token. The execution of ```ls``` happens at most once, for a given token list, though.

## Aliased Instructions

- Aliases are created and destroyed with ```createalias``` and ```destroyalias```. These two instruction tokens are treated specially and can only be executed by themselves.

- If the first token of an instruction is ```createalias```, then the next token is expected to be the alias name and anything that comes after the alias name is considered to be the aliased instruction. This means that instructions like:
```
in-mysh-now>createalias myalias ; 
in-mysh-now>echo one myalias echo two
one
two
in-mysh-now>
```
 are valid.

- Aliases also work **recursively**. For example:
```
in-mysh-now>createalias alias1 echo one
in-mysh-now>createalias anotheralias alias1 ; alias1  
in-mysh-now>anotheralias
one
one
```

- If an alias is destroyed but it is mentioned in another existing alias, the policy is the following:
```
in-mysh-now>destroyalias alias1       
in-mysh-now>anotheralias
alias1: Command Not Found
alias1: Command Not Found
in-mysh-now>
```

- The aliased instruction is stored as a string within a hash table data structure, along with the alias name.

- The function ```replaceAliasesInList()``` finds the tokens of the list that match to aliased instructions. For each token that matches to an aliased instruction, it splits the corresponding aliased instruction into another token list and it recursively calls itself upon the new token list.

- Aliases can be overwritten, without explicitly destroying previous aliases.

## Instruction History

- The instruction history can be viewed with ```myHistory```. 

- An instruction can be called with ```prev <index>``` where ```<index>``` is the id of the instruction, as listed in ```myHistory```. Alternatively, ```prev``` assumes that ```<index>``` is 1.

- Instructions that get executed by ```prev``` are not stored in the history.

- The instructions are stored in a connected list structure, through the ```addToHistory()``` function, defined in ```src/interface.c```. If the capacity of the list reaches a certain threshold (defined in the header file), then the oldest instruction gets dropped from the list, whenever a new instruction is added.  

- ```prev``` works simply by calling ```interpretInstruction()``` on the string stored in the history list.
