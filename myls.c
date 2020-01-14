#include <stdio.h>
#include <stdlib.h>
#include <string.h>
///////////////////////////////////
// myls.c
// Philip Taggart
//
// Custom program to simulate
// ls -l
///////////////////////////////////

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


int main(){
	DIR *currentDr = opendir("./"); //current directory
	struct dirent *dirEntry; //directory struct
	struct stat buf; //struct for file information
	char *filename;
	int check;
	struct passwd *uid; // user owner
	struct group *gid; // group owner
	char tBuf[20]; 
	struct tm * time; // struct for time

	// cycle through all files in directory
	while ((dirEntry = readdir(currentDr)) != NULL){ 
            filename = dirEntry->d_name; // get each individual filename
            if(filename[0]!='.'){
                check = stat(filename, &buf); // retrieve file information
                if (check == 0){
            	   uid = getpwuid(buf.st_uid); // translate user
            	   gid = getgrgid(buf.st_gid); // translate group
            	   time = localtime (&(buf.st_mtime)); // get time
				   strftime(tBuf, 20, "%b %d %H:%M", time); // translate time
            	
            	   //translate permissions and print
                    // Parsing implementation for the permissions was taken from Stack Overflow
            	   printf( (S_ISDIR(buf.st_mode)) ? "d" : "-");
        		   printf( (buf.st_mode & S_IRUSR) ? "r" : "-");
        		   printf( (buf.st_mode & S_IWUSR) ? "w" : "-");
        		   printf( (buf.st_mode & S_IXUSR) ? "x" : "-");
        		   printf( (buf.st_mode & S_IRGRP) ? "r" : "-");
        		   printf( (buf.st_mode & S_IWGRP) ? "w" : "-");
        		   printf( (buf.st_mode & S_IXGRP) ? "x" : "-");
       			   printf( (buf.st_mode & S_IROTH) ? "r" : "-");
        		   printf( (buf.st_mode & S_IWOTH) ? "w" : "-");
        		   printf( (buf.st_mode & S_IXOTH) ? "x" : "-");
    			
    			   // Print all other data
                   printf(" %ld %s %s %7td %s %s", buf.st_nlink, uid->pw_name, gid->gr_name, buf.st_size, tBuf, filename);
    			   printf("\n");
                }   
            }
    }


    closedir(currentDr);
	
	return 0;
}