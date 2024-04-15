//P2-SSOO-23/24

//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

#define MAX_COMMANDS 8
#define MSG_SIZE 100


// files in case of redirection
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}

/* myhistory */

/* myhistory */

struct command
{
  // Store the number of commands in argvv
  int num_commands;
  // Store the number of arguments of each command
  int *args;
  // Store the commands
  char ***argvv;
  // Store the I/O redirection
  char filev[3][64];
  // Store if the command is executed in background or foreground
  int in_background;
};

int history_size = 20;
struct command * history;
int head = 0;
int tail = 0;
int n_elem = 0;

void free_command(struct command *cmd)
{
    if((*cmd).argvv != NULL)
    {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++)
        {
            for (argv = *(*cmd).argvv; argv && *argv; argv++)
            {
                if(*argv){
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
}

void store_command(char ***argvv, char filev[3][64], int in_background, struct command* cmd)
{
    int num_commands = 0;
    while(argvv[num_commands] != NULL){
        num_commands++;
    }

    for(int f=0;f < 3; f++)
    {
        if(strcmp(filev[f], "0") != 0)
        {
            strcpy((*cmd).filev[f], filev[f]);
        }
        else{
            strcpy((*cmd).filev[f], "0");
        }
    }

    (*cmd).in_background = in_background;
    (*cmd).num_commands = num_commands-1;
    (*cmd).argvv = (char ***) calloc((num_commands) ,sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands , sizeof(int));

    for( int i = 0; i < num_commands; i++)
    {
        int args= 0;
        while( argvv[i][args] != NULL ){
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1) ,sizeof(char *));
        int j;
        for (j=0; j<args; j++)
        {
            (*cmd).argvv[i][j] = (char *)calloc(strlen(argvv[i][j]),sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j] );
        }
    }
}


/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

	if (!isatty(STDIN_FILENO))
    {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush (stdin);
			fflush(stdout);
		}
	}

	/*********************************/

	char ***argvv = NULL;
	int num_commands;

	history = (struct command*) malloc(history_size *sizeof(struct command));
	int run_history = 0;    

	while (1) 
	{
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history)
        {
            run_history=0;
        }
        else
        {
            // Prompt 
            write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

            // Get command
            //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
            executed_cmd_lines++;
            if( end != 0 && executed_cmd_lines < end)
            {
                command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
            }
            else if( end != 0 && executed_cmd_lines == end)
                return 0;
            else
                command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/
	    if (command_counter > 0 && command_counter <= MAX_COMMANDS) // Right ammount of commands
        {
            // Code mycalc
            if (strcmp(argvv[0][0], "mycalc") == 0) {
                char msg[MSG_SIZE]; // Where we are going to save the message to be printed
                if (argvv[0][1] != NULL && argvv[0][2] != NULL && argvv[0][3] != NULL){

                    // Convert operators into integers
                    int op1 = atoi(argvv[0][1]);
                    int op2 = atoi(argvv[0][3]);          

                    // We have case 'add'
                    if(strcmp(argvv[0][2], "add") == 0)
                    {
                        char buf[128];
                        char *p = buf; // Pointer of buf
                        if (p == NULL) p = "0";
                        
                        // We save in buf (string) the future value of Acc
                        sprintf(buf, "%d", (atoi(buf) + op1 + op2));

                        // We set the new Acc value
                        if (setenv("Acc", p, 1) < 0) 
                            {
                            perror("Error giving value to environment variable\n");
                            exit(-1);
                            }

                        // Print message
                        sprintf(msg,"[OK] %d + %d = %d; Acc %s\n", op1, op2, op1+op2, getenv("Acc"));
                        fprintf(stderr, msg);
                    }
                    
                    // We have case 'mul'
                    else if(strcmp(argvv[0][2], "mul") == 0)
                    {
                        sprintf(msg,"[OK] %d * %d = %d\n", op1, op2, op1*op2);
                        fprintf(stderr, msg);
                    }

                    // We have case 'div'
                    else if(strcmp(argvv[0][2], "div") == 0)
                    {
                        if(op2 == 0)
                            printf("[ERROR] You cannot divide by 0\n"); // Divisor cannot be 0
                        // Print message
                        else
                        {
                            sprintf(msg,"[OK] %d / %d = %d; Remainder %d\n", op1, op2, op1/op2, op1%op2);
                            fprintf(stderr, msg);
                        }
                    }
                    else
                        // We have error case
                        printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                }
                else
                    // We have error case
                    printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
                
            }


//************************************************************************************************
            // Code myhistory
            else if (strcmp(argvv[0][0], "myhistory") == 0)
            {
                if (argvv[0][1] == NULL) // No argument, print all command history by std_errout
                {
                    char msg[MSG_SIZE]; // Where we are going to save the message to be printed
                    int temphead = head; // Temporal variable to iterate through the history
                    for (int i=0; i < n_elem; i++) // Iterate all the history
                    {
                        sprintf(msg, "%d ", i); // Reset msg and add the number of the command in history order
                        // Iterate to add all commands
                        for (int j=0; j < history[temphead].num_commands; j++) // Iterate all the commands of the history
                        {
                            for (int k=0; k < history[temphead].args[j]; k++) // Iterate all the arguments of the command
                            {
                                if (k == 0) // If it is the first argument
                                    strcat(msg, history[temphead].argvv[j][k]); // Concatenate the command
                                else // If it is not the first argument
                                {
                                    strcat(msg, " "); // Add a space between arguments
                                    strcat(msg, history[temphead].argvv[j][k]); // Concatenate the arguments of the command
                                }
                            }
                            if (j < history[temphead].num_commands-1) // If it is not the last command
                                strcat(msg, " | "); // Add a new vertical line between commands
                        }

                        // Once done with the commands, check for redirections
                        if (strcmp(history[temphead].filev[0], "0") != 0) // Check if there is an input redirection
                        {
                            strcat(msg, " < "); // Add input redirection symbol
                            strcat(msg, history[temphead].filev[0]); // Add the input redirection file
                        }
                        if (strcmp(history[temphead].filev[1], "0") != 0) // Check if there is an output redirection
                        {
                            strcat(msg, " > "); // Add output redirection symbol
                            strcat(msg, history[temphead].filev[1]); // Add the output redirection file
                        }
                        if (strcmp(history[temphead].filev[2], "0") != 0) // Check if there is an error redirection
                        {
                            strcat(msg, " !> "); // Add error redirection symbol
                            strcat(msg, history[temphead].filev[2]); // Add the error redirection file
                        }

                        // Check for background symbol
                        if (history[temphead].in_background == 1) // Check if the command was executed in background
                            strcat(msg, " &"); // Add the background symbol

                        // Move pointer and print message
                        temphead++;
                        if (temphead == history_size) // If we reach the end of the array, go to the beginning
                            temphead = 0;

                        strcat(msg, "\n"); // Add a new line at the end of the message
                        fprintf(stderr, msg); // Print the command by error output
                    }
                }
                else // An argument was given. Change input parameters to the command we want to execute.
                {
                    // Access the command we want to execute
                    int command_number = (head + atoi(argvv[0][1])) % history_size; // Convert the argument to integer
                    int print_command = atoi(argvv[0][1]); // Save the command number to print it later
                    if (command_number < n_elem && command_number >= 0) // Check if the command number is valid
                    {
                        // Change the input parameters to the command we want to execute
                        for (int i=0; i < history[command_number].num_commands; i++) // Iterate all the commands of the history
                        {
                            if (argvv[i] != NULL) // If the command is not NULL
                                free(argvv[i]); // Free the memory of the command
                            argvv[i] = malloc(sizeof(char*) * history[command_number].args[i]); // Allocate memory for the command
                            for (int j=0; j < history[command_number].args[i]; j++) // Iterate all the arguments of the command
                            {
                                argvv[i][j] = strdup(history[command_number].argvv[i][j]); // Change the arguments of the command
                            }
                        }
                        if (history[command_number].args[0] < 2) // If there are less than 2 arguments (only one command)
                        {
                            argvv[0][1] =  NULL; // Set the last argument to NULL to indicate end of arguments.
                                                    // This is needed to avoid executing the command with <command_nummber> as argument 
                        }
                        for (int i=0; i < 3; i++) // Iterate all the redirections
                        {
                            strcpy(filev[i], history[command_number].filev[i]); // Change the redirections
                        }
                        in_background = history[command_number].in_background; // Change the background flag

                        command_counter = history[command_number].num_commands; // Change the number of commands
                        

                        run_history = 1; // Change the flag to run the command
                        char msg[MSG_SIZE]; // Where we are going to save the message to be printed
                        sprintf(msg, "Running command %d\n", print_command); // Print the command number
                        fprintf(stderr, msg); // Print the command by error output
                    }
                    else // Command number is not valid
                    {
                        printf("ERROR: Command not found\n");
                    }
                }
            }

            
            //
            //
//************************************************************************************************


            //Simple commands
            if (strcmp(argvv[0][0], "mycalc") != 0 && strcmp(argvv[0][0], "myhistory") != 0) // It is a simple command or myhistory was executed with a valid argument
            {   
                // Check if the command is being run from myhistory or not
                if (run_history == 0) // If not, add to myhistory
                {
                    // Store commands in history before execution
                    if (n_elem < history_size) // There is space for another command
                    {
                        store_command(argvv, filev, in_background, &(history[tail])); // Store command in history
                        n_elem++;
                    }
                    else if (tail == history_size) // History full. Reached end of the array
                    {
                        if (head == history_size) // If head is at the end of the array, move it to the beginning
                            head = 0;
                        head++; // Point to the next position in history for later printing

                        tail = 0; // Move to the beginning of the array
                        free_command(&(history[tail])); // Free the memory of the command in the beginning of the array
                        store_command(argvv, filev, in_background, &(history[tail])); // Store new command in history
                    }
                    else // History full. Not reached end of the array
                    {
                        free_command(&(history[tail])); // Free the memory of the command in the current position
                        store_command(argvv, filev, in_background, &(history[tail]));

                        head++; // Point to the next position in history for later printing
                    }
                    tail++; // Point to next position in history
                }

                // More than one command (from 2 to n commands.)
                if (command_counter > 1)
                {
                    int original_stderror; // To store stderr
                    // Error redirection
                    if (strcmp(filev[2], "0") != 0) // Check if there is an error redirection
                    {
                        // Store stderr to restore it later
                        original_stderror = dup(STDERR_FILENO);
                        close(STDERR_FILENO); // Close stderr
                        int fd = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0666); // Open file to write
                        if (fd != 2) // If redirection was done correctly, fd should be 2
                        {
                            perror("Error redirecting errors output");
                            exit(-1);
                        }
                    }

                    // Create pipe descriptors.
                    int pip1[2];
                    int pip2[2];

                    // Create pipes before fork. Link inside child process.
                    for (int i=0; i < command_counter; i++)
                    {
                        if (i % 2 == 0) // New pipe 1 since pipe 1 has been fully used, we need a new one. Also, case for i == 0
                        {
                            if (i != command_counter-1) // Check if it is not the last iteration, since we do not need a pipe 1 for the last command.
                            {
                                if (pipe(pip1) == -1) // Create pipe 1
                                {
                                    perror("Error creating pipe");
                                    exit(-1);
                                }
                            }
                        }
                        else // New pipe 2 since pipe 2 has been fully used, we need a new one.
                        {
                            if (i != command_counter-1) // Check if it is not the last iteration, since we do not need a pipe 2 for the last command.
                                if (pipe(pip2) == -1) // Create pipe 2
                                {
                                    perror("Error creating pipe");
                                    exit(-1);
                                }
                        }

                        // Create a child, link pipes and execute the command.
                        pid_t pid = fork();
                        switch (pid)
                        {
                        case -1: // Error creating a child
                            perror("Error creating a child");
                            exit(-1);
                        
                        case 0: // Child process. Will execute the command once linked the pipes

                            // First command
                            if (i == 0)
                            {
                                // Input redirection
                                if (strcmp(filev[0], "0") != 0) // Check if there is an input redirection
                                {
                                    close(STDIN_FILENO); // Close stdin
                                    int fd = open(filev[0], O_RDONLY); // Open file to read
                                    if (fd != 0) // If redirection was done correctly, fd should be 0
                                    {
                                        perror("Error opening file for input redirection");
                                        exit(-1);
                                    }
                                }

                                close(pip1[0]); // Close pipe 1 read descriptor since it is not needed
                                dup2(pip1[1], STDOUT_FILENO); // Redirect stdout to pipe 1 write descriptor
                                close(pip1[1]); // Close duplicated pipe 1 write descriptor
                            }
                            else if (i % 2 == 1) // Odd iteration -> read from pipe 1 and write into pipe 2
                            {
                                if (i == command_counter-1) // Last command. Only read from pipe and write into std_out
                                {
                                    // Output redirection
                                    if (strcmp(filev[1], "0") != 0) // Check if there is an output redirection
                                    {
                                        close(STDOUT_FILENO); // Close stdout
                                        int fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0666); // Open file to write
                                        if (fd != 1) // If redirection was done correctly, fd should be 1
                                        {
                                            perror("Error redirecting output");
                                            exit(-1);
                                        }
                                    }

                                    dup2(pip1[0], STDIN_FILENO); // Redirect stdin to pipe 1 read descriptor
                                    close(pip1[0]); // Close duplicated pipe 1 read descriptor
                                }
                                else // Not the last command. Read and write with pipes
                                {
                                    close(pip2[0]); // Close pipe 2 read descriptor since it is not needed
                                    dup2(pip1[0], STDIN_FILENO); // Redirect stdin to pipe 1 read descriptor
                                    close(pip1[0]); // Close duplicated pipe 1 read descriptor
                                    dup2(pip2[1], STDOUT_FILENO); // Redirect stdout to pipe 2 write descriptor
                                    close(pip2[1]); // Close duplicated pipe 2 write descriptor
                                }
                            }
                            else // Even iteration -> read from pipe 2 and write into pipe 1
                            {
                                if (i == command_counter-1) // Last command. Only read from pipe and write into std_out
                                {
                                    // Output redirection
                                    if (strcmp(filev[1], "0") != 0) // Check if there is an output redirection
                                    {
                                        close(STDOUT_FILENO); // Close stdout
                                        int fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0666); // Open file to write
                                        if (fd != 1) // If redirection was done correctly, fd should be 1
                                        {
                                            perror("Error redirecting output");
                                            exit(-1);
                                        }
                                    }
                                    
                                    dup2(pip2[0], STDIN_FILENO); // Redirect stdin to pipe 2 read descriptor
                                    close(pip2[0]); // Close duplicated pipe 2 read descriptor
                                }
                                else // Not the last command. Read and write with pipes
                                {
                                    close(pip1[0]); // Close pipe 1 read descriptor since it is not needed
                                    dup2(pip2[0], STDIN_FILENO); // Redirect stdin to pipe 2 read descriptor
                                    close(pip2[0]); // Close duplicated pipe 2 read descriptor
                                    dup2(pip1[1], STDOUT_FILENO); // Redirect stdout to pipe 1 write descriptor
                                    close(pip1[1]); // Close duplicated pipe 1 write descriptor
                                }
                            }

                            // Execute the command
                            getCompleteCommand(argvv, i);
                            execvp(argv_execvp[0], argv_execvp);
                            perror("Error executing the command"); // This line should not be executed
                            exit(-1);

                        default: // Parent process, does not execute other process.
                        
                            // Close pipes for each iteration.
                            if (i == 0) // First iteration, only close write end of pipe 1
                            {
                                close(pip1[1]);
                            }
                            else if (i % 2 == 1) // Odd iteration, check if it is the last command and close pipes accordingly
                            {
                                if (i == command_counter-1) // Last command, close read end of pipe 1
                                {
                                    close(pip1[0]);
                                }
                                else // Not the last command, close both pipes
                                {
                                    close(pip1[0]); // Close read end of pipe 1
                                    close(pip2[1]); // Close write end of pipe 2
                                }
                            }
                            else // Even iteration, check if it is the last command and close pipes accordingly
                            {
                                if (i == command_counter-1) // Last command, close read end of pipe 2
                                {
                                    close(pip2[0]);
                                }
                                else // Not the last command, close both pipes
                                {
                                    close(pip2[0]);
                                    close(pip1[1]);
                                }
                            }
                            if (i == command_counter-1) // Only for last child created (the one that prints the output)
                            {
                                if (in_background == 0) // Foreground: wait for child to finish.
                                {
                                    while (wait(&status) != pid) // Wait for child to finish. Close other processes in the way
                                        continue;
                                }
                                else // Background: print pid and do not wait for child.
                                    printf("%d\n", pid); // Do not wait for child. Print pid of proccess in background
                                
                                // Restore stderr
                                if (strcmp(filev[2], "0") != 0) // Check if there was an error redirection
                                {
                                    close(STDERR_FILENO); // Close file descriptor of the redirected error file
                                    dup(original_stderror); // Restore stderr
                                    close(original_stderror); // Close duplicated file descriptor
                                }

                            }   
                        }
                    }
                }

                // Only one command, no pipes needed
                else
                {
                    // Create a child and execute the process
                    pid_t pid = fork();
                    switch (pid)
                    {
                    case -1: // Error while creating a child
                        perror("Error creating a child");
                        exit(-1);
                    
                    case 0: // Child process

                        // Input redirection
                        if (strcmp(filev[0], "0") != 0) // Check if there is an input redirection
                        {
                            close(STDIN_FILENO); // Close stdin
                            int fd = open(filev[0], O_RDONLY); // Open file to read
                            if (fd != 0) // If redirection was done correctly, fd should be 0
                            {
                                perror("Error opening file for input redirection");
                                exit(-1);
                            }
                        }

                        // Output redirection
                        if (strcmp(filev[1], "0") != 0) // Check if there is an output redirection
                        {
                            close(STDOUT_FILENO); // Close stdout
                            int fd = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0666); // Open file to write
                            if (fd != 1) // If redirection was done correctly, fd should be 1
                            {
                                perror("Error redirecting output");
                                exit(-1);
                            }
                        }

                        // Error redirection
                        if (strcmp(filev[2], "0") != 0) // Check if there is an error redirection
                        {
                            close(STDERR_FILENO); // Close stderr
                            int fd = open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0666); // Open file to write
                            if (fd != 2) // If redirection was done correctly, fd should be 2
                            {
                                perror("Error redirecting errors output");
                                exit(-1);
                            }
                        }

                        // Execute command
                        getCompleteCommand(argvv, 0);
                        execvp(argv_execvp[0], argv_execvp);
                        perror("Error executing the command"); // This line should not be executed
                        exit(-1);

                    default: // Parent process
                        if (in_background == 0)
                            while (wait(&status) != pid) // Wait for child to finish. Close other processes in the way
                                continue;
                        else
                            printf("%d\n", pid); // Do not wait for child. Print pid of proccess in background

                    }
                }
            }


        }
        else if (command_counter > MAX_COMMANDS)
        {
            printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
        }

    }
    // Return and finish
	return 0;
}
