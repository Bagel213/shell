///////////////////////////////////
// myshell.c
// Philip Taggart
//
// Command prompt loop with 
// function calls and shell 
// exit functionality
/////////////////////////////////// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void cmdLoop(); 			 // myshell.c
extern int external(char *); // shellCmdFunctions.c
extern void check(); 		 // shellCmdFunctions.c
extern void killAll();	     // shellCmdFunctions.c
extern void waitAll();       // shellCmdFunctions.c
extern int mySet(char *);
extern int myCd(char *);
extern int myPwd();

// Loop for command line input with check for built in functions
void cmdLoop(){
	
	char line[80]; // Input size 80
	int status = 1; // maintain loop
	char *exit = "exit";
	char *set = "set";
	char *pwd = "pwd";
	char *cd = "cd";
	
	// do loop to continue to run command line and check for exit and ctrl-d
	do{												
		check(); //check for zombies using a linked list defined in external.c
		printf("$ ");
		
		// Get input.  If NULL, Ctrl-D pressed = exit shell
		if(fgets(line, sizeof(line), stdin) == NULL){
			status = 0;
			killAll(); // Kill all remaining background processes
			printf("All background processes killed.\n  Thanks for using myshell!\n");
			fflush(stdin);
			fflush(stdout);
			_exit(EXIT_SUCCESS);
		}
		
		else if (strncmp(line, exit, 4) == 0 && strlen(line) == 5){
			status = 0;
			printf("Waiting for all background processes to complete.\n");
			waitAll(); // wait for all remaining background process
			printf("Thanks for using myshell!\n");
			fflush(stdin);
			fflush(stdout);
			sleep(1);
			_exit(EXIT_SUCCESS);
		}

		else if (strncmp(line, set, 3) == 0)
			status = mySet(line);

		else if (strncmp(line, cd, 2) == 0)
			status = myCd(line);

		else if (strncmp(line, pwd, 3) == 0)
			status = myPwd();

		// Handle all other command promt input
		else
			status = external(line);	

	} while (status); // continue loop after command returns
		
	
	return;
}

// shell entry
int main(){
	printf("Welcome to myshell!  Coded by Philip Taggart\n");
	cmdLoop();
return 0;
}