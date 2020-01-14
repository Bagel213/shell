///////////////////////////////////
// builtin.c
// Philip Taggart
//
// Built in functions pwd, cd, and
// set 
/////////////////////////////////// 

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Print current directory
int myPwd(){
	
	char absPath[200];
	//printf("Current Path = ");
	if (getcwd(absPath, sizeof(absPath)) == NULL) {
		perror("Error getting absolute path.\n");
		return 1;
	}
	
	printf("%s\n", absPath);
	return 1;
}

// change directory
int myCd(char *line){
	char **tokens = malloc(strlen(line)*sizeof(char*));
  char *tkn;
  int i = 0;
  char *path;
  char *home;
    
   // If returned pressed after typing "cd" go to home directory
   if (strlen(line)==3){
      home = getenv("HOME");
      if (chdir(home) < 0) 
        perror("chdir");
    }

    else{
      // Parse command line input
      tkn = strtok(line, " \n\r\t\a");

    	while(tkn != NULL){
      	tokens[i] = tkn;
      	i++;
      	tkn = strtok(NULL, " ");
    	}
    	tokens[i] = NULL;
  	
  	path = tokens[1];
  	
  	path[strcspn(path, "\n")] = 0; // remove /n from path
  	
  	// change directory
  	if (chdir(path) < 0) 
  		perror("chdir");

  	free(tokens);
  }
	return 1;
}

// set new environment variable or add to existing
int mySet(char *line){
	
    char **tokens = malloc(strlen(line)*sizeof(char*));
  	char *tkn;
    int i = 0; 
    char *current;
    
    // Parse command line input
    tkn = strtok(line, "=");

  	while(tkn != NULL){
    	tokens[i] = tkn;
    	i++;
    	tkn = strtok(NULL, "=");
  	}
  	tokens[i] = NULL;
  	 
  	// If incorrect input 	
  	if (tokens[1] == NULL){
  		printf("Incorrect format. No environment variable set.\n");
  		return 1;
  	}


 // If MYPATH exists add new path to current with strcat (adds to front of list)
  	else if ((current = getenv("MYPATH")) != NULL){
  		//current[strcspn(current, "\n")] = 0;
  		tokens[1][strcspn(tokens[1], "\n")] = 0;
  		char *newPath = malloc((2*strlen(current))*sizeof(char)); //create string to pass to setenv
  		newPath = tokens[1];
  		strcat(newPath, ":");
		  strcat(newPath, current); 		
      setenv("MYPATH",newPath, 1);
  	}
  	// If MYPATH does not exit, create with enV
  	else{
  		setenv("MYPATH",tokens[1], 1);
  	}
  	
  	//Display MYPATH after editing
  	printf("MYPATH = %s", getenv("MYPATH"));

  	free(tokens);
  	
  	return 1;
}