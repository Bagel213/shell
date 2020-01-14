myshell README
Philip Taggart

Installation: After unpacking the .tar file, install myshell by typing "make" at the prompt. This command will make the executable myshell and the executable myls from the .c files provided in the tar file.

Running myshell:  To run myshell ensure the program is executable and type ./myshell at the command prompt to run.  You will receive the welcome message and a new command prompt, $.  

***All input at the command prompt is limited to 80 characters max***

Built in Commands:
pwd: type pwd at the command prompt to view the current working directory

cd: type cd and the desired directory to change directories. ie. cd /dir/dir2
If the user enters cd at the command prompt and immediately presses return the directory will be set to home.

set: To set a new environment variable, or edit an existing variable type.
ie. set MYPATH=/dir/dir where MYPATH is the desired variable name.  Of note, the MYPATH variable is the only only variable other than the PATH variable that is used by this shell.  Using set on an existing environment variable will append the new directory onto the beginning of the list of directories with each directory separated by a colon(:).

exit:  There are two ways to exit myshell.  Typing exit and pressing Ctrl-d. Both exit the shell with different functionality (see background processes below).

External commands:
Single command:  To run a command such as ls, just type ls at the command prompt with any desired flags.

Pipes:  This shell will accept up to 200 commands with piping, limited by the 80 characters allowed at the command prompt.  To pipe use the | symbol.  ie. ls | grep.o | wc

Redirection: Single redirection is as follows: cmd > file, cmd < file.  For redirection of input and output use cmd < file1 > file2.  No other combination of redirection arrows is supported.

Background process:  Background processes are supported by this shell by adding & as the final argument to any command.  ie. sleep 10 & or sleep 10&.  Background processes that have ended will be removed from the process table prior every command prompt.  All background processes that have not had STDIN redirected by piping, etc, will have their STDIN redirected to /dev/null.
All background process still running when exiting the shell will be allowed to complete prior to exiting if exit is typed at the command prompt (**if the process is awaiting input or has an infinite loop the shell may never exit with this method**).  If Ctrl-d is used at the prompt then all processes will be killed before exiting.

myls: To run myls it must first be added to MYPATH in order to run.  As with the PATH variable, this allows myls to run in any directory.  myls is a simulation of ls -l.  The only difference is myls only displays the contents of the current directory.  It does not take any arguments to display the contents of other directories in the current implementation.

*Some code was reused from other sources and is labeled as such in the comments.  The code is found in shellCmdFunction.c and myls.c.  The code was taken from simpleshell.c coded by Philip Taggart in shellCmdFunction for parsing input, and from Stack Overflow in myls.c for printing of file permissions.