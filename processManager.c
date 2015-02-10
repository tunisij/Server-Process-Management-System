#include <pthread.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_STR_LEN 512
#define NUM_COMMANDS 5
#define MAX_CHILDREN 256

/**********************************************************************
 * (Descripton)
 *
 * Author: John Tunisi
 *********************************************************************/

void sighandler(int signum);
void createServer(char * serverName, int minProcs, int maxProcs);
void abortServer(char * serverName);
void createProcess(char * serverName);
void abortProcess(char * serverName);
void displayStatus();
bool parseCommand(char * command);

int numServers;
int numProcesses;
pid_t childPid[MAX_CHILDREN];
char *childName[MAX_CHILDREN];

/**********************************************************************
 * Main method used for the execution of the Process Management
 * System.
 *********************************************************************/
int main(){
	signal(SIGINT, sighandler);
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);
	char * command;
	numServers = 0;
	numProcesses = 0;

	while(1){
		command = (char *)malloc(MAX_STR_LEN * sizeof(char));
		fgets(command, MAX_STR_LEN, stdin); 
		if(!parseCommand(command)){
			continue;
		}
	}
	free(command);
	return 0;
}


/**********************************************************************
 * Parses the command received from the user
 *
 * Params:	command:	The string of characters inputted by the user
 *********************************************************************/
bool parseCommand(char * cmd){
	strtok(cmd, "\n");
	char *pch = strtok(cmd, " ");
	char *commandList[NUM_COMMANDS] = {"createserver", "createprocess", "abortserver", "abortprocess", "displaystatus"};

	//help	
	if(strstr(cmd, "-help")){
		char *commandArgs[NUM_COMMANDS] = {"<MIN_PROCESSES> <MAX_PROCESSES> <SERVERNAME>",
		   	"<SERVERNAME>", "<SERVERNAME>", "<SERVERNAME>", "<NONE>"};
		printf("Commands list:\n");
		int i;
		for(i = 0; i < NUM_COMMANDS; i++){
			printf("%s\t%s\n", commandList[i], commandArgs[i]);
		}
	}
	//createserver
	else if(!strcmp(cmd, commandList[0])){
		int minProcs, maxProcs;
		char * serverName;
		pch = strtok(NULL, " ");
		if(pch != NULL){
			minProcs = atoi(pch);
		}
		pch = strtok(NULL, " ");
		if(pch != NULL){
			maxProcs = atoi(pch);
		}
		pch = strtok(NULL, " ");
		if(pch != NULL){
			serverName = pch;
			printf("\nServer Name: %s\nminProcs: %d\nmaxProcs: %d\n\n", serverName, minProcs, maxProcs);
			createServer(serverName, minProcs, maxProcs);
		}
	}
	//create process
	else if(!strcmp(cmd, commandList[1])){
		pch = strtok(NULL, " ");
		char *serverName = pch;
		createProcess(serverName);
	}
	//abort server
	else if(!strcmp(cmd, commandList[2])){
		pch = strtok(NULL, " ");
		char *serverName = pch;
		abortServer(serverName);
	}
	//abort process
	else if(!strcmp(cmd, commandList[3])){
		pch = strtok(NULL, " ");
	//	char *serverName = pch;
		//abortProcess(serverName);
	}
	//display status
	else if(!strcmp(cmd, commandList[4])){
		displayStatus();
	}
	else{
		printf("Invalid command. Type -help for a list of commands\n");
		return false;
	}
	return true;

	//	printf("cmd: %s\n", userCommand.command);
	//	printf("miniprocs: %d\n", userCommand.miniProcs);
	//	printf("maxprocs: %d\n", userCommand.maxProcs);
	//	printf("name: %s\n", userCommand.serverName);
}

/**********************************************************************
 * Handles an interrupt signal
 *
 * Params:	signum:		The argument of a received signal
 *********************************************************************/
void sighandler(int signum){
	//child exits
	if(signum == SIGUSR1){
		printf("[SERVER]: I am exiting.\n");
		exit(0);
	}
	else if(signum == SIGUSR2){
	}
	//terminates entire program
	else if(signum == SIGINT){
		exit(0);	
	}
}

/**********************************************************************
 * Creates a server	by forking a process
 *
 * Params:	minProcs: 	The minimum number of processes that can be
 * 		  				handled at once
 * 		  	maxProcs: 	The maximum number of processes that can be
 * 		  				handled at once
 * 		  	serverName: The name of the server to create
 *********************************************************************/
void createServer(char *serverName, int minProcs, int maxProcs){
	pid_t pid;
	if((pid = fork()) < 0){ //error
		perror("Fork failure");
		exit(1);
	}
	else if(pid == 0){ //child
		int i;
		for(i = 0; i < minProcs; i++){
		    sprintf(serverName, "server %d", i);
			printf("Process created: %s\n", serverName);
			createProcess(serverName);
		}
		printf("\n");
		sleep(100);
		//if done, signal to parent
		//	kill(getppid(), SIGUSR1);
		//	exit(0);
	}
	else{ //parent
		childPid[numServers] = pid;
		childName[numServers] = serverName;
		numServers++;
	}
//	printf("\nCreated server.\nMIN_PROCESSES:\t%d\nMAX_PROCESSES:\t%d\nSERVER_NAME:\t%s\n", cmd.minProcs, cmd.maxProcs, cmd.serverName);
}


/**********************************************************************
 * Aborts a specified server. Any children will be aborted as well.
 *
 * Params:	serverName: The name of the server to be aborted 
 *********************************************************************/
void abortServer(char * serverName){
	//strtok(serverName, "\n");
	int status;
	int i;
	for(i = 0; i < numServers; i++){
		if(!strcmp(childName[i], serverName)){
			kill(childPid[i], SIGUSR1);
			wait(&status);
		}
	}	
	numServers--;
}


/**********************************************************************
 * Creates a process for a given server
 *
 * Params:	serverName: The name of the parent server to which this
 * 						process will become a child
 *********************************************************************/
void createProcess(char * serverName){
	numProcesses++;
}


/**********************************************************************
 * Aborts a process for a given server
 *
 * Params:	serverName: The name of the parent server to which a
 * 						process will be aborted
 *********************************************************************/
void abortProcess(char * serverName){
	numProcesses--;
}


/**********************************************************************
 * Displays the current state of the Process Management System and
 * all child servers and processes
 *********************************************************************/
void displayStatus(){
	printf("Servers running: %d\n", numServers);
	printf("Child processes running: %d\n", numProcesses); 
	printf("\n");
}
