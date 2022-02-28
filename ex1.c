//#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define MAX_CMD_SIZE 100

int main(int argc, char *argv[], char **envp){
	//I define the prompt to be no larger than 10 characters and the cmd entered by a user to be no more than 100
	//These are arbitrary values and may need to be increased
	char prompt[10];
	char cmd[100];

	//getout is the variable I use to track if the program should exit
	int getout = 0;
	
	//Check if -p PROMPT was passed into the program
	if( argc >= 3 && strcmp(argv[1],"-p") == 0){
		strcpy(prompt,argv[2]);
	}
	else{
		strcpy(prompt,"308sh>");
	}

	//Enter the main loop of my shell
	while(!getout){
		//Prompt for command
		printf("%s",prompt);
		//Get input from user
		fgets(cmd, MAX_CMD_SIZE, stdin);
		//Remove the newline and carage return
		cmd[strcspn(cmd,"\n\r")] = 0;
		int index;
	        int background = 0;
		char *ret;
		char firstcmd[100];
		int argcount = 1;
		//Get string up to the first space
		ret = strchr(cmd, ' ');
		//While there is more data to process
		while(ret != NULL){
			//Count number of arguments
			argcount++;
			ret = strchr(ret+1, ' ');	
		}
		//Allocate an array to store pointers to strings
		char **arglist;
		arglist = malloc( (argcount * sizeof(char *))+2 );
		int tmpi = 0;
		char *last = cmd;
		char *curr;
		//For loop to split the command into argv array
		for(tmpi=0; tmpi<argcount; tmpi++){
			int size;
			//Malloc a space in the array for the new string
			arglist[tmpi] = (char *)malloc(100);
			curr = strchr(last,' ');
			if(curr == NULL){
				size = strlen(last);
			}
			else{
				size = (int)(curr - last);
			}
			//Copy the string into the array
			strncpy(arglist[tmpi],last,size);
			last = curr+1;
		}
		arglist[argcount] = NULL;
		//If the last argument is an & meaning we want to background it
        	if(strcmp(arglist[argcount-1],"&") == 0)
		{
			background = 1;arglist[argcount-1] = NULL;
		}
		//Check for the basic pid,ppid,cd,pwd,exit commands
		if(strcmp(arglist[0], "pid") == 0){
			printf("%d\n",getpid());	
		}
		else if(strcmp(arglist[0],"ppid") == 0){
			printf("%d\n",getppid());	
		}
		else if(strcmp(arglist[0],"cd") == 0){
			if(argcount < 2){				
				chdir(getenv("HOME"));
			}
			else{
				chdir(arglist[1]);
			}	
		}
		else if(strcmp(arglist[0],"pwd") == 0){
			char tmpcwd[100];
			getcwd(tmpcwd, 100);
			printf("%s\n",tmpcwd);	
		}
		else if(strcmp(arglist[0],"exit") == 0){
			getout = 1;
		}
		//If nothing was entered just move on to the next prompt
		else if(strcmp(arglist[0],"") == 0){}
		//If nothing else matches start searching the path
		else{
			int ret;
			//Create a child process to handle the new function call
			ret = fork();
			int stat;
			int waitstat;
			//If we are the parent process
			if(ret != 0){
				//Print the new process ID
				printf("[%d] %s\n",ret, arglist[0]);
				fflush(stdout);
				//If we don't want to background it waitpid(-1) to stall
				if(!background){
                 		   waitpid(-1,&waitstat,0);
				  printf("[%d] %s Exited %d\n",ret, arglist[0], waitstat);
				  fflush(stdout);
		                }
				//If we do want to background call it with WNOHANG
				else{
		                  waitpid(-1,&waitstat,WNOHANG);
                		}
		       }
          	       //If we are the child process
		       else{
				//Call execvp to try and find the command in the path
				stat = execvp(arglist[0], arglist);
				//If there was an error in execvp handle it
				if(stat < 0){ printf("Cannot exec %s: No such file or directory\n",arglist[0]); exit(1);}
	   		        if(background){printf("[%d] %s Exited %d\n",getpid(),arglist[0], stat);}
            		}
			
	    }
	//Check if a background process exited since last command was run
        int tmpretstat;
        int test = waitpid(-1,&tmpretstat,WNOHANG);
	//If a process has exited print out it's pid and status
	if(test > 0){
          //int bgstat;
          waitpid(test,NULL,WUNTRACED);
          printf("[%d] Exited %d \n",test,tmpretstat);
        }
        //Null out all the old string references and free all the needed memory
        strcpy(cmd,"\0");
        background = 0;
		for(tmpi=0;tmpi<argcount;tmpi++){
			arglist[tmpi] = NULL;
			free(arglist[tmpi]);
		}
		free(arglist);
	}
}
