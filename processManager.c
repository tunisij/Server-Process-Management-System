#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define MAX_STR_LEN 512

/**********************************************************************
 * (Descripton)
 *
 * Author: John Tunisi
 *********************************************************************/

void sighandler(int signum);
void createServer(int minProcs, int maxProcs, char * serverName);
void abortServer(char * serverName);
void createProcess(char * serverName);
void abortProcess(char * serverName);
void displayStatus();
void getInput(char * input);
void parseCommand(char * command);



/**********************************************************************
 * Main method used for the execution of the Process Management
 * System.
 *********************************************************************/
int main(){
	signal(SIGINT, sighandler);
	char * command;
	input = (char *)malloc(MAX_STR_LEN * sizeof(char));

	while(1){
		getInput(command);
		parseCommand(command);
	}

	return 0;
}


/**********************************************************************
 * Gets input from the user
 *
 * Params:	input:		The string of characters inputted by the user
 *********************************************************************/
void getInput(char * input){
	fgets(input, MAX_STR_LEN, stdin); 
}


/**********************************************************************
 * Parses the command received from the user
 *
 * Params:	command:	The string of characters inputted by the user
 *********************************************************************/
void parseCommand(char * command){
	printf("C: %s\n", command);
}


/**********************************************************************
 * Handles an interrupt signal
 *
 * Params:	signum:		The argument of a received signal
 *********************************************************************/
void sighandler(int signum){
	if(signum == SIGINT){
		exit(0);	
	}
}


/**********************************************************************
 * Creates a server	
 *
 * Params:	minProcs: 	The minimum number of processes that can be
 * 		  				handled at once
 * 		  	maxProcs: 	The maximum number of processes that can be
 * 		  				handled at once
 * 		  	serverName: The name of the server to create
 *********************************************************************/
void createServer(int minProcs, int maxProcs, char * serverName){

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
