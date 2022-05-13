#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

struct Cmd{
    char * command;
    char ** args;
    char * inputfilename;
    char * outputfilename;
    int * pipeTo; //use to store whether pipe will be used
    int * pipeFrom;
} Cmd;

struct CmdSet{
    struct Cmd * CmdArray;
    int flag;
} CmdSet;

int isOperator(char * token){
    if(strcmp(token, "<") == 0){    
        return 1;
    }
    else if(strcmp(token, ">") == 0){
        return 2;
    }
    else if(strcmp(token, ">>") == 0){ 
        return 3;
    }
    else if(strcmp(token, "&") == 0){
        return 4;
    }
    else{
        return 0;
    }
}

/* ERRORS TO CATCH:

"Error: Usage: %s [prompt]\n", program_name

"Error: \"&\" must be last token on command line\n"@

"Error: Ambiguous input redirection.\n"

"Error: Missing filename for input redirection.\n"@

"Error: Ambiguous output redirection.\n"

"Error: Missing filename for output redirection.\n"@

"Error: Invalid null command.\n"

"Error: open(\"%s\"): %s\n", file_name, strerror(errno)"

*/
void * process_command(struct CmdSet * Cmdset, int * exit_code){
    char * command = malloc(1024 * sizeof(char));
    if((command = fgets(command, 1024, stdin)) == NULL){
        *exit_code = 0;
        return NULL;
    }
    char ** tokens = get_tokens(command);
    free(command);

    int i = 0;
    int numPipes = 0;
    int counter = 0;
    if(strcmp(tokens[0], "exit") == 0){
        *exit_code = 0;
        return NULL;
    } 
    while(tokens[i] != NULL){
        if(tokens[i] == "|"){
            numPipes++;
        }
        i++;
    }
    counter = i;
    Cmdset->CmdArray = malloc((numPipes + 1) * sizeof(Cmd));
    
    i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    while(tokens[i] != NULL){
        if(k == 0){
            Cmdset->CmdArray[j].command = (char *)malloc(sizeof(char *));
            Cmdset->CmdArray[j].command = tokens[i];
            Cmdset->CmdArray[j].args = (char **)malloc(10 * sizeof(char *));
            Cmdset->CmdArray[j].inputfilename = (char *)malloc(sizeof(char *));
            Cmdset->CmdArray[j].outputfilename = (char *)malloc(sizeof(char *));
            Cmdset->CmdArray[j].pipeTo = (int *)malloc(sizeof(int));
            Cmdset->CmdArray[j].pipeFrom = (int *)malloc(sizeof(int));
            i++;
        }

        if(tokens[i] == "|"){ // is pipe
            if(tokens[i+1] != NULL && strcmp(tokens[i+1],"|") == 0){
                fprintf(stderr, "Error: Invalid null command.\n");
                *exit_code = 1;
                return NULL;
            }
            Cmdset->CmdArray[j].pipeTo = 1;
            j++;
            Cmdset->CmdArray[j].pipeFrom = 1;
            k = 0;
            l = 0;
        }
        else{
            k++;
        }
        if (tokens[i] == NULL){
            return NULL;
        }
        int operator = isOperator(tokens[i]);
        if(operator != 0){
            i++;
            switch(operator){
                case 1:
                    if(i >= counter){
                        fprintf(stderr, "Error: Missing filename for input redirection.\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    if(Cmdset->CmdArray[j].inputfilename != NULL){
                        fprintf(stderr, "Error: Ambiguous input redirection.\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    Cmdset->CmdArray[j].inputfilename = tokens[i];
                    break;
                case 2:
                    if(i >= counter){
                        fprintf(stderr, "Error: Missing filename for output redirection.\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    if(Cmdset->CmdArray[j].outputfilename != NULL){
                        fprintf(stderr, "Error: Ambiguous output redirection.\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    Cmdset->CmdArray[j].outputfilename = tokens[i];
                    break;
                case 3:
                    if(i >= counter){
                        fprintf(stderr, "Error: Missing filename for output redirection.\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    if(Cmdset->CmdArray[j].outputfilename != NULL){
                        fprintf(stderr, "Error: Ambiguous output redirection.\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    Cmdset->CmdArray[j].outputfilename = tokens[i];
                    break;
                case 4:
                    if(i < counter){
                        fprintf(stderr, "Error: \"&\" must be last token on command line\n");
                        *exit_code = 1;
                        return NULL;
                    }
                    Cmdset->flag = 1;
                    break;
            }
            i++;
        }
        else{
            //an argument to the command
            if(i < counter){
                Cmdset->CmdArray[j].args[l] = tokens[i];
                l++;
            }
        }
        i++;
    }
    free_tokens(tokens);
}

int main(int argc, char *argv[]){
    while(1){
        printf("mysh: ");
        //process all command lines
        struct CmdSet * Cmdset = malloc(sizeof(struct Cmdset *));
        int * exit_code = malloc(sizeof(int *));
        *exit_code = 2;
        process_command(Cmdset, exit_code);

        //check command:


        if(*exit_code == 0){
            exit(0);
        }
        else if(*exit_code == 1){ //if the command line is malformed
            continue;
        }
        else{
            int numCmds = (int)(sizeof(Cmdset->CmdArray) / sizeof(Cmd));
            int * pids = malloc(numCmds * sizeof(int));
            int i = 0;
            while(i < numCmds){ //for each Cmd in CmdSet
                int child_pid = fork();
                pids[i] = child_pid;
                //store pid in array
                if(child_pid == 0){
                    int counter = 0;
                    while(Cmdset->CmdArray[i].args[counter] != "\0"){
                        counter++;
                    }
                    if (execvp(Cmdset->CmdArray[i].args[0], Cmdset->CmdArray[i].args) == -1) {
                        perror("execvp()");
                        break;
                    }
                    //redirection
                    if(Cmdset->CmdArray[i].inputfilename != NULL){
                        FILE * fd;
                        if((fd = open(Cmdset->CmdArray[i].inputfilename, O_RDONLY)) == -1){
                            fprintf(stderr, "Error: open(\"%s\"): %s\n", Cmdset->CmdArray[i].inputfilename, strerror(errno));
                            continue;
                        }
                        dup2(fd, 0);
                        close(fd);
                    }
                    if(Cmdset->CmdArray[i].outputfilename != NULL){
                        FILE * fd;
                        if((fd = open(Cmdset->CmdArray[i].inputfilename, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1){
                            fprintf(stderr, "Error: open(\"%s\"): %s\n", Cmdset->CmdArray[i].outputfilename, strerror(errno));
                            continue;
                        }
                        dup2(fd, 1);
                        close(fd);
                    }
                    execvp(Cmdset->CmdArray[i].args[0], Cmdset->CmdArray[i].args);
                }
                else{
                    if(Cmdset->flag == 1){
                        //background
                        wait(child_pid);
                        continue;
                    }
                    //run the parent process
                }    
                i++;
            }
            //if foreground, traverse pid array and execute all child processes, then next command line
            //it should get rid 
            //if background, then it only stores pid array, does nothing
            free(pids);
        }

        int i = 0;
        int j = 0;
        while(i < 1){
            while(j < 10){
                free(Cmdset->CmdArray[i].args[j]);
                j++;
            }
            free(Cmdset->CmdArray[i].command);
            free(Cmdset->CmdArray[i].inputfilename);
            free(Cmdset->CmdArray[i].outputfilename);
            free(Cmdset->CmdArray[i].pipeTo);
            free(Cmdset->CmdArray[i].pipeFrom);
            i++;
        }
        
        free(Cmdset->flag);
        free(Cmdset->CmdArray);
        free(Cmdset);
        free(exit_code);
    }
    return 0;
}