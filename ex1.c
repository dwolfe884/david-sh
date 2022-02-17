//#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define MAX_CMD_SIZE 100

int main(int argc, char *argv[], char **envp){
	char prompt[10];
	char cmd[100];
	int getout = 0;
	if( argc >= 3 && strcmp(argv[1],"-p") == 0){
		strcpy(prompt,argv[2]);
	}
	else{
		strcpy(prompt,"308sh>");
	}
	while(!getout){
		printf("%s",prompt);
		//scanf("%24s",cmd);
		fgets(cmd, MAX_CMD_SIZE, stdin);
		cmd[strcspn(cmd,"\n\r")] = 0;
		int index;
        int background = 0;
		char *ret;
		char firstcmd[100];
//		char * arglist[] = {NULL};
		int argcount = 1;
		ret = strchr(cmd, ' ');
		while(ret != NULL){
			argcount++;
			ret = strchr(ret+1, ' ');	
		}
		char **arglist;
		arglist = malloc( (argcount * sizeof(char *))+1 );
		int tmpi = 0;
		char *last = cmd;
		char *curr;
//		printf("Entering arg loop with count %d\n", argcount);	
		for(tmpi=0; tmpi<argcount; tmpi++){
			int size;
			arglist[tmpi] = (char *)malloc(100);
			//printf("parsing this: %s\n", last);
			curr = strchr(last,' ');
			if(curr == NULL){
				size = strlen(last);
			}
			else{
				size = (int)(curr - last);
			}
			//printf("curr: %p \n last: %p \n",curr,last);
			//printf("Adding: %d\n",size);
			strncpy(arglist[tmpi],last,size);
			last = curr+1;
		}
		arglist[argcount] = NULL;
		//printf("arguments: \n");
		//for(tmpi=0;tmpi<argcount;tmpi++){printf("%s\n",arglist[tmpi]);}
        if(strcmp(arglist[argcount-1],"&") == 0){background = 1;arglist[argcount-1] = NULL;}
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
		else if(strcmp(arglist[0],"") == 0){}
		else{
			int ret;
			ret = fork();
			int stat;
			int waitstat;
			if(ret != 0){
				printf("[%d] %s\n",ret, arglist[0]);
				fflush(stdout);
				if(!background){
                  waitpid(-1,&waitstat,0);
				  printf("[%d] %s Exited %d\n",ret, arglist[0], waitstat);
				  fflush(stdout);
                }else{
                  waitpid(-1,&waitstat,WNOHANG);
                }
			}
			else{
				stat = execvp(arglist[0], arglist);
				if(stat < 0){ printf("%s errored: %d\n",arglist[0],stat); exit(1);}
			    if(background){printf("[%d] %s Exited %d\n",getpid(),arglist[0], stat);}
            }
			
		}
        int tmpretstat;
        int test = waitpid(-1,&tmpretstat,WNOHANG);
		if(test > 0){
          //int bgstat;
          waitpid(test,NULL,WUNTRACED);
          printf("[%d] Exited %d \n",test,tmpretstat);
        }
        //printf("this -> %d\n", test);
        strcpy(cmd,"\0");
        background = 0;
		for(tmpi=0;tmpi<argcount;tmpi++){
			arglist[tmpi] = NULL;
			free(arglist[tmpi]);
		}
		free(arglist);
	}
}
