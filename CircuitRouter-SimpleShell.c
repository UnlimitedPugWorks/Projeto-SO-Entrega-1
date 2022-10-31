#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "lib/commandlinereader.h"
#include "CircuitRouter-SimpleShell.h"
/* Main */
int main(int argc, char** argv){
	int MAXCHILDREN = NOMAXCHILDREN, numargs, Pid, currentPid, finished = 0, status, i = 0;
	processlist PIDlist = NULL, current;
	char **line = (char**)malloc(MAXPARAMS*sizeof(char*));
	char *buffer = (char*)malloc(BUFFERSIZE* sizeof(char));
	if(argc == 2){
		MAXCHILDREN = atoi(argv[1]); /*Se existe um argumento, então é o MAXCHILDREN*/
	}
	prompt(MAXCHILDREN);
	while(finished == 0){
		printf("%s",">");
		numargs = readLineArguments(line, MAXPARAMS, buffer, BUFFERSIZE);
		if ((numargs == 1) && (strcmp(line[0],EXITCOMMAND) == 0)){
			finished = 1;
		}
		else if((numargs == 2) && (strcmp(line[0],RUNCOMMAND) == 0)){
			if ((MAXCHILDREN == NOMAXCHILDREN) || ((MAXCHILDREN != NOMAXCHILDREN)&&(activeProcess(PIDlist)<MAXCHILDREN))){
				Pid = runSeqSolver(line);
				PIDlist = insertEnd(PIDlist, Pid);
			}
			else{
				printf("%s\n%s\n","MAXCHILDREN reached", "Processing...");
				while(activeProcess(PIDlist) == MAXCHILDREN){
						/*Fica a espera enquanto regista*/
				}
				printf("%s\n", "Processed! Processes will now be launched");
				Pid = runSeqSolver(line);
				PIDlist = insertEnd(PIDlist, Pid);
			}
		}
		else{
			printf("%s\n","Command not recognized");
		}
	}
	current = PIDlist;
	while(current != NULL){
		currentPid = current->PID;
		if (current->finished !=FINISHED){ /*If a process is not finished, wait for it*/
			waitpid(currentPid, &status,0);
			current->finished = FINISHED;
			current->finishedstatus = WEXITSTATUS(status);
		}
		/*else{ If it is finished, get the status it finished with
			status = current->finishedstatus;
		}*/
		printf("CHILD EXITED(PID=%d; return %s)", currentPid, (current->finishedstatus==0)?"OK":"NOK"); /*If that status is equal to 0, it exited in an okay form, otherwise it ended*/
		current = current->next;
	}
	freelist(PIDlist);
	free(buffer);
	for(i = 0; i < numargs;i++){
		free(line[i]);
	}
	free(line);
	printf("%s\n", "END.");
	return 0;
}
int runSeqSolver(char** args){
	int SeqPid = fork();
	char* argumentos[] = {FILENAME, args[1], NULL};
	if(SeqPid == 0){ /*Child Process*/
		execv(PATH, argumentos);/*Executes the program*/
	}
	return SeqPid;/*The parent returns the child's PID*/
}

int activeProcess(processlist list){ 
	processlist current = list;
	int currentPid, counter = 0, status;
	while(current != NULL){
		if (current->finished != FINISHED){
			currentPid = current->PID;
			waitpid(currentPid, &status, WNOHANG);
			if(WIFEXITED(status)){ 
				current->finished = FINISHED;
				current->finishedstatus = WEXITSTATUS(status);
			}
			else{
				counter++;
			}
		}
		current = current->next;
	}
	return counter;
}

void prompt(int MAXCHILDREN){
	printf("Welcome to CircuitRouter-SimpleShell.\n");
	if (MAXCHILDREN == NOMAXCHILDREN){
		printf("MAXCHILDREN: Unlimited\n");
	}
	else{
		printf("MAXCHILDREN: %d\n", MAXCHILDREN);
	}
}