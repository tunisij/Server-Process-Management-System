#include <sys/resource.h>

#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>

#define MAX_STR_LEN 512
#define NUM_COMMANDS 5

/**********************************************************************
 * (Descripton)
 *
 * Author: John Tunisi
 *********************************************************************/

void sighandler(int signum);
pid_t createServer(int minProcs, int maxProcs, char * serverName);
void abortServer(char * serverName);
void createProcess(char * serverName);
void abortProcess(char * serverName);
void displayStatus();
struct userCommand parseCommand(char * command);
void executeCommand(struct userCommand);

struct userCommand{
	char *command;
	char *serverName;
	int miniProcs;
	int maxProcs;
	bool valid;
};

/**********************************************************************
 * Main method used for the execution of the Process Management
 * System.
 *********************************************************************/
int main(){
	signal(SIGINT, sighandler);
	signal(SIGUSR1, sighandler);
	signal(SIGUSR2, sighandler);

	char * command;
	struct userCommand userCommand;
	userCommand.valid = false;
	pid_t webServer, fileServer;

	while(1){
		command = (char *)malloc(MAX_STR_LEN * sizeof(char));
		fgets(command, MAX_STR_LEN, stdin); 
		userCommand = parseCommand(command);
		if(!userCommand.valid){
			free(command);
			continue;
		}
		executeCommand(userCommand);
		free(command);
	}
	return 0;
}


/**********************************************************************
 * Executes the appropriate function based on the user command
 *
 * Params:	c:	A userCommand struct containing inputted info
 *********************************************************************/
void executeCommand(struct userCommand c){
	if(!strcmp(c.command, "createserver")){
		createServer(c.miniProcs, c.maxProcs, c.serverName);
	}
	else if(!strcmp(c.command, "abortserver")){
		abortServer(c.serverName);
	}
	else if(!strcmp(c.command, "createprocess")){
		createProcess(c.serverName);
	}
	else if(!strcmp(c.command, "abortprocess")){
		abortProcess(c.serverName);
	}
	else if(!strcmp(c.command, "displaystatus")){
		displayStatus();
	}else{
		printf("false!\n");
	}
}


/**********************************************************************
 * Parses the command received from the user
 *
 * Params:	command:	The string of characters inputted by the user
 *********************************************************************/
struct userCommand parseCommand(char * command){
	char *commandList[NUM_COMMANDS] = {"createserver", "createprocess", "abortserver", "abortprocess", "displaystatus"};
	struct userCommand userCommand;

	//help	
	if(strstr(command, "-help")){
		char *commandArgs[NUM_COMMANDS] = {"<MIN_PROCESSES> <MAX_PROCESSES> <SERVERNAME>", "<SERVERNAME>", "<SERVERNAME>", "<SERVERNAME>", "<NONE>"};
		printf("Commands list:\n");
		int i;
		for(i = 0; i < NUM_COMMANDS; i++){
			printf("%s\t%s\n", commandList[i], commandArgs[i]);
		}
		userCommand.valid = false;
		return userCommand;
	}
	
	char *pch = strtok(command, " ");
	//createserver
	if(!strcmp(command, commandList[0])){
		userCommand.command = commandList[0];
		pch = strtok(NULL, " ");
		if(pch != NULL)
			userCommand.miniProcs = atoi(pch);
		pch = strtok(NULL, " ");
		if(pch != NULL)
			userCommand.maxProcs = atoi(pch);
		pch = strtok(NULL, " ");
		if(pch != NULL){
			userCommand.serverName = pch;
			userCommand.valid = true;
		}
	}
	//createprocess
	else if(!strcmp(command, commandList[1])){
		userCommand.command = commandList[1];
		pch = strtok(NULL, " ");
		if(pch != NULL){
			userCommand.serverName = pch;
			userCommand.valid = true;
		}
	}
	//abortserver
	else if(!strcmp(command, commandList[2])){
		userCommand.command = commandList[2];
		pch = strtok(NULL, " ");
		if(pch != NULL){
			userCommand.serverName = pch;
			userCommand.valid = true;
		}
	}
	//abortprocess
	else if(!strcmp(command, commandList[3])){
		userCommand.command = commandList[3];
		pch = strtok(NULL, " ");
		if(pch != NULL){
			userCommand.serverName = pch;
			userCommand.valid = true;
		}
	}
	//displaystatus
	else if(strstr(command, commandList[4])){
		userCommand.command = commandList[4];
		userCommand.valid = true;
	}
	else{
		userCommand.valid = false;
		printf("Invalid command. Type -help for a list of commands\n");
	}
//	printf("cmd: %s\n", userCommand.command);
//	printf("miniprocs: %d\n", userCommand.miniProcs);
//	printf("maxprocs: %d\n", userCommand.maxProcs);
//	printf("name: %s\n", userCommand.serverName);
	return userCommand;
}


/**********************************************************************
 * Handles an interrupt signal
 *
 * Params:	signum:		The argument of a received signal
 *********************************************************************/
void sighandler(int signum){
	int status;
	struct rusage usage;
	pid_t pid;
	if(signum == SIGUSR1){
		wait4(pid, &status, 0, &usage);
		printf("Child process terminated\n");
	}
	else if(signum == SIGUSR2){
		exit(0);
	}
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
pid_t createServer(int minProcs, int maxProcs, char * serverName){
	pid_t pid;
	
	if((pid = fork()) < 0){ //error
		perror("Fork failure");
		exit(1);
	}
	else if(pid == 0){ //child
		int i;
		for(i = 0; i < minProcs; i++){
			sleep(10);	
			//if done, signal to parent
			kill(getppid(), SIGUSR1);
			exit(0);
		}
	}
	else{ //parent

	}
	printf("\nCreated server.\nMIN_PROCESSES:\t%d\nMAX_PROCESSES:\t%d\nSERVER_NAME:\t%s\n", minProcs, maxProcs, serverName);
	return pid;
}


/**********************************************************************
 * Aborts a specified server. Any children will be aborted as well.
 *
 * Params:	serverName: The name of the server to be aborted 
 *********************************************************************/
void abortServer(char * serverName){
		
}


/**********************************************************************
 * Creates a process for a given server
 *
 * Params:	serverName: The name of the parent server to which this
 * 						process will become a child
 *********************************************************************/
void createProcess(char * serverName){

}


/**********************************************************************
 * Aborts a process for a given server
 *
 * Params:	serverName: The name of the parent server to which a
 * 						process will be aborted
 *********************************************************************/
void abortProcess(char * serverName){

}


/**********************************************************************
 * Displays the current state of the Process Management System and
 * all child servers and processes
 *********************************************************************/
void displayStatus(){

}
