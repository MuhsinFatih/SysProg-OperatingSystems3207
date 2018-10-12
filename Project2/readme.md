Note: This file can be viewed as a markdown document  
Note 2: Documentation includes portions of GNU bash documentation as some of the functionality are very similar (https://linux.die.net/man/1/bash)

# mufash - man page

## Name
mufash - mufa-shell

## Synopsis
```
[command] [<argument> ...]
```

## Description
mufash is a command language interpreter that executes commands read from the standard input or from a file, inpired by the GNU shell: bash.

## Shell Grammar
### **Simple Commands**
A simple command is a sequence of optional variable assignments followed by blank-separated words and redirections, and terminated by a control operator. The first word specifies the command to be executed, and is passed as argument zero. The remaining words are passed as arguments to the invoked command.

### **Lists**
Commands may be seperated by delimiter semicolon (;) or newline

### **Pipelines**
A pipeline is a sequence of one or more commands separated by the control operator | . The format for a pipeline is:
```
[command] [<arguments> ...] | [command] [<arguments> ...]
```
This operator connects the standard output of the command on the left to the standard input of the command on the right.

### **Redirection**
Before a command is executed, its input and output may be redirected using a special notation interpreted by the shell. Redirection may also be used to open and close files for the current shell execution environment. The following redirection operators may precede or appear anywhere within a simple command or may follow a command. Redirections are processed in the order they appear, from left to right.


**Output redirection (> or >>)**
```
[command] [<arguments> ...] > [file]

[command] [<arguments> ...] >> [file]
```
The operator `>` redirects standard output of the command to given file. If the file does not exist, creates the file. If the file exists, truncates the file to zero length before the redirection

The operator `>>` on the other hand, does not truncate the file to zero length, instead appends to the file

**Input redirection (<)**
```
[command] [<arguments> ...] < [file]
```
The operator `<` redirects the given file to the standard input of the command.

### Background execution
```
[command] [<arguments> ...] &
```
The operator `&` runs the given command in background

## Built-in commands:
```
cd:              change directory  
clr:             clear terminal screen  
ls (dir):        list directory: Supports multiple arguments  
environ:         list environment variables  
echo:            print a string to std_out  
help:            display this message  
pause:           waits until user presses enter  
quit (exit):     quit terminal  
```

## Other supported features
* **Escaping non-alphanumeric characters is possible:**
`\[char]`. For example: "\ " can be used to escape the whitespace character (space).
* **Comments are supported:** Use `#` for comments. Any input after this character is ignored until the next line
* **Quote symbol ('') is supported:** You can enter argument without escaping characters
* **Home symbol is supported:** Use `~` to change directory to a relative directory to the home folder
* **History is supported:** Use up and down arrow keys on supported terminal clients to access history, use tab to autocomplete and double tap to list autocomplete options