#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#define MAX_STR_LEN 512
#define NUM_COMMANDS 5
#define MAX_CHILDREN 256


/**********************************************************************
 * Simulates a server process management system.
 * A server is forked off and then creates copies of itself based
 * on the minimum number of processes provided.
 * 
 * The manager has the following capabilities:
 * createServer
 * abortServer
 * createProcess
 * abortProcess
 * displayStatus
 *
 * Author: John Tunisi
 *********************************************************************/

void sighandler(int signum);
void createServer(char * serverName, int minProcs, int maxProcs);
void abortServer(char * serverName);
void createProcess();
void abortProcess();
void displayStatus();
bool parseCommand(char * command);

int numActive;
int numProcesses;
int totalServers;
pid_t childPid[MAX_CHILDREN];
char *childName[MAX_CHILDREN];
pthread_mutex_t lock;
int min_processes;
int max_processes;
char *myName;
char *serverList[MAX_CHILDREN];


/**********************************************************************
 * Main method used for the execution of the Process Management
 * System.
 *********************************************************************/
int main(){
	signal(SIGINT, sighandler);
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);
	char * command;
	numActive = 0;
	numProcesses = 0;
	totalServers = 0;
	min_processes = -1;
	max_processes = -1;
	srand(time(NULL));

	if(pthread_mutex_init(&lock, NULL) != 0){
		printf("mutex failed init\n");
		return 1;
	}

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
 * Params:	cmd:	The string of characters inputted by the user
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
			if(minProcs < 0 || maxProcs < 0){
				printf("Cannot be below 0\n\n");
				return false;
			}
			if(maxProcs < minProcs){
				printf("MaxProcs must be greater than MinProcs!\n\n");
				return false;
			}
		}
		pch = strtok(NULL, " ");
		if(pch != NULL){
			int i;
			for(i = 0; serverList[i]; i++){
				if(!strcmp(pch, serverList[i])){
					printf("Cannot reuse server names!\n\n");
					return false;
				}
			}
			myName = serverName = pch;
			printf("\nServer Name: %s\nminProcs: %d\nmaxProcs: %d\n\n", serverName, minProcs, maxProcs);
			createServer(serverName, minProcs, maxProcs);
		}
	}
	//create process
	else if(!strcmp(cmd, commandList[1])){
		pch = strtok(NULL, " ");
		int i;
		for(i = 0; i < totalServers; i++){
			if(!strcmp(childName[i], pch)){
				kill(childPid[i], SIGUSR2);
				return true;
			}
		}
		printf("\nCould not add a process for that server\n");
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
		int i;
		for(i = 0; i < totalServers; i++){
			if(!strcmp(childName[i], pch)){
				kill(childPid[i], SIGUSR1);
				return true;
			}
		}
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
}

/**********************************************************************
 * Handles an interrupt signal
 *
 * Params:	signum:		The argument of a received signal
 *********************************************************************/
void sighandler(int signum){
	//parent to child: abort a process
	if(signum == SIGUSR1){
		abortProcess();
		numActive--;
	}
	//parent to child: create a process
	else if(signum == SIGUSR2){
		createProcess();
		numActive++;
		totalServers++;
	}
	//terminates entire program
	else if(signum == SIGINT){
		int i, status;
		for(i = 0; i < totalServers; i++){
			if(!strcmp(childName[i], myName)){
				kill(childPid[i], SIGINT);
				wait(&status);
			}
		}	
		printf("I am exiting.\n");
		pthread_mutex_lock(&lock);
		numActive--;
		pthread_mutex_unlock(&lock);
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
		perror("Fork failure\n");
		exit(1);
	}
	else if(pid == 0){ //child
		myName = serverName;
		min_processes = minProcs;
		max_processes = maxProcs;
		int i;
		for(i = 0; i < minProcs; i++){
			createProcess(serverName);
		}
		pause();
	}
	else{ //parent
		childPid[totalServers] = pid;
		childName[totalServers] = serverName;
		pthread_mutex_lock(&lock);
		serverList[totalServers] = serverName;
		totalServers++;
		numActive++;
		pthread_mutex_unlock(&lock);
	}
}


/**********************************************************************
 * Aborts a specified server. Any children will be aborted as well.
 *
 * Params:	serverName: The name of the server to be aborted 
 *********************************************************************/
void abortServer(char * serverName){
	int i;
	int status;
	for(i = 0; i < totalServers; i++){
		if(!strcmp(childName[i], serverName)){
			//kill(childPid[i], SIGUSR1);
			kill(childPid[i], SIGINT);
			wait(&status);
		}
	}	
	pthread_mutex_lock(&lock);
	numActive--;
	pthread_mutex_unlock(&lock);
}


/**********************************************************************
 * Creates a process for the current server
 *********************************************************************/
void createProcess(){
	if(numActive == max_processes){
		printf("Cannot create more processes!\n");
		return;
	}

	pid_t pid;
	if((pid = fork()) < 0){ //error
		perror("Fork failure\n");
		exit(1);
	}
	else if(pid == 0){ //child
		printf("Process added\n");
		pause();
	}
	else{ //parent
		childPid[totalServers] = pid;
		childName[totalServers] = myName;
		pthread_mutex_lock(&lock);
		totalServers++;
		numActive++;
		pthread_mutex_unlock(&lock);
	}
}


/**********************************************************************
 * Aborts a process for the current server
 *********************************************************************/
void abortProcess(){
	int i, status;
	printf("serverName: %s\n", myName);
	for(i = 0; i < MAX_CHILDREN; i++){
		if((!strcmp(myName, childName[i])) && (getpid() != childPid[i])){
			if(!(numActive > min_processes)){
				printf("Cannot abort process!\n");
				return;
			}
			kill(childPid[i], SIGINT);
			wait(&status);
			break;
		}
	}
	pthread_mutex_lock(&lock);
	numActive--;
	pthread_mutex_unlock(&lock);
}


/**********************************************************************
 * Displays the current state of the Process Management System
 *********************************************************************/
void displayStatus(){
	printf("Original servers running: %d\n", numActive);
	printf("\n");
}
