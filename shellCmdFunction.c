///////////////////////////////////
// shellCmdFunction.c
// Philip Taggart
//
// All functionality for external
// programs.  Allows for multiple
// pipes and redirects as well
// as background processes managed
// with a linked list.
///////////////////////////////////
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

struct bkgndNode{
	struct bkgndNode *next;
	int pid;
};

void redirect(char *);
void pipes(char *);
char *searchMYPATH(char*);
void addNode(int);
void check();
void removeNode(struct bkgndNode *);
void killAll();
void waitAll();
extern int myCd(char *);	 // builtin.c
extern int myPwd(); 		 // builtin.c
extern int mySet(char *); 	 // builtin.c

//////////////////////////////////Linked list for background processes//////////////////////////////////
typedef struct bkgndNode *node;
node ptr = NULL;

void addNode(int data){
	node temp = (node)malloc(sizeof(struct bkgndNode));
	//If list empty create first node
	if (ptr == NULL){
		temp->pid = data;
		temp->next = NULL;
		ptr = temp;
		temp = NULL;
		return;
	}
	
	// add node to non-empty list
	else{
		node itr;
		temp->pid = data;
		temp->next = NULL;
		itr = ptr;
		while(itr->next!=NULL)
			itr=itr->next;
		itr->next = temp;
		temp = NULL;
	}	
	return;
}

//Check for ended backgrond processes and delete their nodes
void check(){

	if(ptr == NULL)
		return;
	
	int wpid, st;
	node itr = ptr;
	node tempN;
	
	//Find any children that have changed states with WNOHANG and remove from list
	wpid = waitpid(-1, &st, WNOHANG);
	
	// loop until wpid == 0/no child has changed states
	itr=ptr;
	while (wpid > 0){
		itr=ptr;
		
		// beginning of list is to be removed
		if (itr->pid == wpid){ 
				ptr=ptr->next;
				free(itr);
		}
		
		// node that is not beginning of list to be removed
		else if (itr->next != NULL){
			do{
				if(itr->next->pid == wpid){
					tempN = itr->next;
					itr->next = tempN->next;
					free(tempN);
					tempN=NULL;
				}
				if (itr->next!=NULL)
					itr = itr->next;
			}while (itr->next!=NULL);
		}
		wpid = waitpid(-1, &st, WNOHANG);
	}
	return;
}

void waitAll(){
	if(ptr == NULL)
		return;
	
	int wpid, st;
	node itr = ptr;
	node tempN;
	
	//Find any children that have changed states with WNOHANG and remove from list
	wpid = waitpid(-1, &st, WNOHANG);
	
	// loop until wpid == 0/no child has changed states
	itr=ptr;
	while (ptr != NULL){
		itr=ptr;
		
		// beginning of list is to be removed
		if (itr->pid == wpid){ 
				ptr=ptr->next;
				free(itr);
		}
		
		// node that is not beginning of list to be removed
		else if (itr->next != NULL){
			do{
				if(itr->next->pid == wpid){
					tempN = itr->next;
					itr->next = tempN->next;
					free(tempN);
					tempN=NULL;
				}
				if (itr->next!=NULL)
					itr = itr->next;
			}while (itr->next!=NULL);
		}
		wpid = waitpid(-1, &st, WNOHANG);
	}
	return;
}

//Kill all remaining processes prior to exit
void killAll(){
	if (ptr!=NULL){
		node itr = ptr;
		do{
		
			kill(itr->pid, SIGKILL);
			itr = itr->next;

		}while (itr!=NULL);
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////// check for pipes or redirect or run single command///////////////////////////
int external(char *line){
	
	char **tokens = malloc(strlen(line)*sizeof(char*));
  	char **arguments = malloc(strlen(line)*sizeof(char*));
  	char *tkn;
    int i = 0;
    int flag = 0;
    int pid;
    int st;
    char *path;
    int bkFlag = 0;
	
	// check for pipe or redirect
	for (i=0; i<strlen(line); i++){
		if (line[i] == '|'){
			pipes(line);
			flag = 1;
		}
		else if (line[i] == '<' || line[i] == '>'){
			redirect(line);
			flag = 1;
		}
	}
	if (flag == 0){ // no redirect or pipe
			
			//check for &
			if ((line[strlen(line)-2]) == '&'){
				bkFlag = 1;
				line[strlen(line)-2] = '\0';
			}
			else bkFlag = 0;
			
			// Parsing taken from OS simpleshell project by Philip Taggart
			i = 0;
			tkn = strtok(line, " \n\r\t\a");

	  		while(tkn != NULL){
	    		tokens[i] = tkn;
	    		tokens[i][strcspn(tokens[i], "\n")] = 0; //check for \n
	    		i++;
	    		tkn = strtok(NULL, " ");
	  		}
	  		tokens[i] = NULL;
	  		  		
			// Run command
			if ((pid=fork()) == 0){
	  			// If background process redirect STDIN so all input is to shell.
	  			if(bkFlag==1)
	  				freopen("/dev/null","r",stdin);
	  			if (execvp(*tokens, tokens) == -1){
					// If not found in system PATH search MYPATH env variable
					path = searchMYPATH(tokens[0]);
					path[strcspn(path, "\n")] = 0;
					if (execv(path, tokens) == -1)
						perror("exec error\n");
				}
				exit(0);
			}
			else{
				//wait based on whether or not process is background
				if(bkFlag==0){
					do{
						waitpid(pid, &st, WUNTRACED);
	      			} while (!WIFEXITED(st) && !WIFSIGNALED(st));
				}
				//if background process add node to list
				else addNode(pid);  	
	    	}
	    	free(arguments);
	    	free(tokens);
	      
	    }
  return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////// Search MYPATH for specified command/////////////////////////
char *searchMYPATH(char *cmd){
	
	char *path = getenv("MYPATH");
	if(path == NULL){
		return cmd;
	}
	path[strcspn(path, "\n")] = 0;
	char **tokens = malloc(strlen(path)*sizeof(char*));
	char *catCmd = malloc(strlen(path)*sizeof(char *));
	char *tkn;
	int i = 0;
	
	tkn = strtok(path, ":");

	// Parse MYPATH and concat on / and cmd to search directories wth access.
  	while(tkn != NULL){ 
    	tokens[i] = tkn;
    	strcpy(catCmd,tokens[i]);
    	strcat(catCmd, "/");
    	strcat(catCmd, cmd);
    	if(access(catCmd, X_OK)==0) // access returns 0 if command is found in dir
  			return strcat(catCmd,"\0");
    	i++;
    	tkn = strtok(NULL, ":");
  	}
  	tokens[i] = NULL;
  	free(tokens);
  	free(catCmd);
  	return cmd;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// Redirection//////////////////////////////////////////////////
void redirect(char *line){
	
		char **tokens = malloc(strlen(line)*sizeof(char*));
		char *tkn;
		char *redirect[2];
		char *filename[2];
		int i=0;
		int r=0;
		int pid, st;
		char *path;
		int fd1, fd2, bkFlag=0;

		// Find redirects and assign to redirect array
		for (i=0;i<strlen(line);++i){
			if (line[i]=='>'){
				redirect[r]=">";
				r++;
			}
			else if (line[i]=='<'){
				redirect[r]="<";
				r++;
			}
		}

		i = 0;
		tkn = strtok(line, " ><");

	  	while(tkn != NULL){
	    	tokens[i] = tkn;
	    	tokens[i][strcspn(tokens[i], "\n")] = 0; //check for \n
	    	i++;
	    	tkn = strtok(NULL, " ><");
	    
	  	}
	  	tokens[i] = NULL;
	  	
	  	// Breakout filenames
	  	if (r==1){
	  		filename[0] = tokens[i-1];
	  		tokens[i-1] = NULL;
	  	}	
	  	else if (r==2){
	  		filename[0] = tokens[i-2];
	  		tokens[i-2] = NULL;
	  		filename[1] = tokens[i-1];
	  		tokens[i-1] = NULL;
	  	}

	  	i=0;
	  	while (tokens[i] != NULL){
	  		//printf("%s\n", tokens[i]);
	  		i++;
	  	}

  		// Check for background flag
  		if (strcmp(tokens[i-1],"&")==0){
  			tokens[i-1] = NULL;
  			bkFlag=1;
  		}
  		else if (tokens[0][strlen(tokens[0])-1] == '&'){
  			tokens[0][strlen(tokens[0])-1]='\0';
  			bkFlag=1;
  		}
  		else if (tokens[i-1][strlen(tokens[0])]=='&'){
  			tokens[i-1][strlen(tokens[0])] = '\0';
  			bkFlag=1;
  		}	
  		
  		if (r==1){

	  		if(strcmp(redirect[0],">")==0){	
	  			if ((pid=fork()) == 0){
	  				if(bkFlag==1)
	  					open("/dev/null", O_RDONLY);
	  				// open file and redirect std out
	  				close(STDOUT_FILENO);
	  				if ((fd1 = open(filename[0], O_WRONLY|O_CREAT|O_TRUNC, 0700)) < 0)
	  					perror("file error");
	  				
	  				//// check for built in functions ////
	  				if(strcmp(tokens[0],"pwd")==0)
    					myPwd();
    				
    				else if(strcmp(tokens[0],"set")==0)
    					mySet(tokens[1]);
    				
    				else if(strcmp(tokens[0],"cd")==0)
    					myCd(tokens[1]);	
	  				
	  				else if (execvp(*tokens, tokens) == -1){
						// If not found in system PATH search MYPATH env variable
						path = searchMYPATH(tokens[0]);
						path[strcspn(path, "\n")] = 0;
						if (execv(path, tokens) == -1)
							perror("exec error\n");
					}
					exit(0);
				}
				else{
					if(bkFlag == 0){
						do{
							waitpid(pid, &st, WUNTRACED);
	      				} while (!WIFEXITED(st) && !WIFSIGNALED(st));
	      			}
	      			else addNode(pid); 
	      		}
	      	}
	      	else if(strcmp(redirect[0],"<")==0){	
	  			if ((pid=fork()) == 0){
	  				// open file and redirect std in
	  				close(STDIN_FILENO);
	  				if ((fd1 = open(filename[0], O_RDONLY, 0700)) < 0)
	  					perror("file error");
	  				
	  				//// check for built in functions ////
	  				if(strcmp(tokens[0],"pwd")==0)
    					myPwd();
    				
    				else if(strcmp(tokens[0],"set")==0)
    					mySet(tokens[1]);
    				
    				else if(strcmp(tokens[0],"cd")==0)
    					myCd(tokens[1]);

	  				if (execvp(*tokens, tokens) == -1){
						// If not found in system PATH search MYPATH env variable
						path = searchMYPATH(tokens[0]);
						path[strcspn(path, "\n")] = 0;
						if (execv(path, tokens) == -1)
							perror("exec error\n");
					}
					exit(0);
				}
				else{
					if(bkFlag == 0){
						do{
							waitpid(pid, &st, WUNTRACED);
	      				}while (!WIFEXITED(st) && !WIFSIGNALED(st));
	      			}
	      			//if background process add node to list
	      			else addNode(pid); 
	      		}
	      	}
	    }
	    else if(r==2){
	    	if ((strcmp(redirect[0],"<")==0) && (strcmp(redirect[1],">")==0)){
	    		
	    		if ((pid=fork()) == 0){
	  				//open both files and redirect stdin or stdout as appropriate
	  				close(STDIN_FILENO);
	  				if ((fd1 = open(filename[0], O_RDONLY, 0700)) < 0)
	  					perror("file error");
	  				close(STDOUT_FILENO);
	  				if ((fd2 = open(filename[1], O_WRONLY|O_CREAT|O_TRUNC, 0700)) < 0)
	  					perror("file error");

	  				//// check for built in functions ////
	  				if(strcmp(tokens[0],"pwd")==0)
    					myPwd();
    				
    				else if(strcmp(tokens[0],"set")==0)
    					mySet(tokens[1]);
    				
    				else if(strcmp(tokens[0],"cd")==0)
    					myCd(tokens[1]);

	  				else if (execvp(*tokens, tokens) == -1){
						// If not found in system PATH search MYPATH env variable
						path = searchMYPATH(tokens[0]);
						path[strcspn(path, "\n")] = 0;
						if (execv(path, tokens) == -1)
							perror("exec error\n");
					}
					exit(0);
				}
				else{
					if(bkFlag == 0){
						do{
							waitpid(pid, &st, WUNTRACED);
	      				} while (!WIFEXITED(st) && !WIFSIGNALED(st));
	      			}
	      			else addNode(pid); 

	  			}
	    	}
	    }
	free(tokens);
	return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////// Pipes ///////////////////////////////////////////////////////

void pipes(char *line){
	
	int numCmds, i, j;
	char **tokens = malloc(strlen(line)*sizeof(char*));
  	char **args = malloc(strlen(line)*sizeof(char*));
  	char *tkn;
  	int st, tpid;
  	char *path;
  	int p=0;
  	int bkFlag = 0;
  	int flag = 0;
  	int  pid[200]; //max 200 processes in list for wait

  	//parse input with |
  	i=0;
	tkn = strtok(line, "|");
  	while(tkn != NULL){
    	tokens[i] = tkn;
    	i++;
    	tkn = strtok(NULL, "|");
   	}
  	numCmds = i;
  	tokens[i] = NULL;
  	
  	// Create pipe array based on number of cmds
  	int fds[numCmds][2];
  	pipe(fds[0]);

  	//check for & at end of cmd/arguments
  	if ((tokens[0][strlen(tokens[0])-2]) == '&'){
		flag = 1;
		tokens[0][strlen(tokens[0])-2] = '\0';
	}

  	// Parse input with " "
  	i=0;
	free(tkn);
	tkn = strtok(tokens[0], " ");
  	while(tkn != NULL){
    	args[i] = tkn;
    	args[i][strcspn(args[i], "\n")] = 0;
    	i++;
    	tkn = strtok(NULL, " ");
   	}
  	args[i] = NULL;
  	
  	//check if backround process
  	if (flag == 1){
  		bkFlag = 1;
  		}
  		  	
  	// initial fork
  	if ((tpid = fork()) == 0){
  		if(bkFlag==1)
	  		freopen("/dev/null","r", stdin);
  		dup2(fds[0][1], STDOUT_FILENO); // redirect pipe write
    	close(fds[0][1]);
    	close(fds[0][0]);
  		if (execvp(*args, args) == -1){
  			path = searchMYPATH(args[0]);
			path[strcspn(path, "\n")] = 0;
				if (execv(path, args) == -1)
					perror("exec error\n");
			
  		}
		_exit(0);
	}
	else {
		// if not background process add to wait list
		if (bkFlag == 0){
  			pid[p] = tpid;
  			p++;}
  		//if background process add node to list
  		else addNode(tpid); 
	}
	
	bkFlag = 0;
	flag = 0;
	///////////////////////Loop for multiple pipes////////////////////////////
	for (j=1; j<numCmds; j++){
   			
   			//check for & at end of cmd/arguments
   			if ((tokens[j][strlen(tokens[j])-2]) == '&'){
				tokens[j][strlen(tokens[j])-2] = '\0';
				flag = 1;
			}

   			//parse input with " "
   			i=0; 
			free(tkn);
			tkn = strtok(tokens[j], " \n");
  			while(tkn != NULL){
    			args[i] = tkn;
    			args[i][strcspn(args[i], "\n")] = 0;
    			i++;
    			tkn = strtok(NULL, " \n");
   			}
  			args[i] = NULL;


  			//check if background process
  			if (flag==1){
  				bkFlag = 1;
  			}

  			
  			// If required (this is not last cmd) create another pipe
			if ((j+1)!=numCmds)
	    		pipe(fds[j]);
			
  			
  			if ((tpid = fork()) == 0){
   					
   					dup2(fds[j-1][0], STDIN_FILENO); // redirect pipe read
    				close(fds[j-1][1]); // close pipe in child
    				close(fds[j-1][0]);
    				
    				// If not the last command use new pipe
    				if ((j+1)!=numCmds){
    					dup2(fds[j][1], STDOUT_FILENO); // redirect pipe write
    					close(fds[j][1]); // close new pipe in child
    					close(fds[j][0]); // close  new pipe in child
    				}

    				//// check for built in functions ////    		
    				if(strcmp(args[0],"pwd")==0)
    					myPwd();
    				
    				else if(strcmp(args[0],"set")==0)
    					mySet(args[1]);
    				
    				else if(strcmp(args[0],"cd")==0)
    					myCd(args[1]);
    			
    				else if (execvp(*args, args) == -1){
						path = searchMYPATH(args[0]);
    					path[strcspn(path, "\n")] = 0;
						if (execv(path, args) == -1)
							perror("exec error\n");
					}
					_exit(0);
			}
			else{
				// if not background process add to wait list
				if(bkFlag==0){
  					pid[p] = tpid;
  					p++;
  				}
  				//if background process add node to list
  				else addNode(tpid); 
				//close old pipe in parent
				close(fds[j-1][1]); 
				close(fds[j-1][0]);
				//reset flags
				bkFlag = 0;
				flag=0;
			}	
	} 
	/////////////////////////////////////////////////////////////////////////
 	
	// wait for all processes in wait list (non-background processes)
 	for (i=0; i<p; i++){

 	do{
			waitpid(pid[i], &st, WUNTRACED);
      	} while (!WIFEXITED(st) && !WIFSIGNALED(st));
 	}
 	free(tokens);
 	free(args);
 	return;

}