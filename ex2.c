// Gal Snir 313588279

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <malloc.h>
#include <stdlib.h>

#define MAX 512

//This struct will represent a process
typedef struct
{
    pid_t pid;
    char parms[MAX];
}process;

//This function split a command to it's arguments and execute it using execvp function
int exec(char *command_list);

int main ()
{
    pid_t pid;
    int status;
    int is_background = 0; // This variable will tell the code if an command supposed to run in the background
    int jobs_len = 0;
    char prev_dir[MAX] = "undefined"; // This variable will save the path of the previous directory
    char curr_dir[MAX]; // This variable will save the path of the current directory
    char buffer[MAX]; // This buffer will hbe use to hold the user input
    int quote_mark = 0;
    // These are 3 temporal string i'll use during the program
    char temp[MAX];
    char temp1[MAX];
    char temp2[MAX];
    process jobs_list[MAX]; // This is a list of process that will hold all the processes running at the moment
    while (1)
    {
        printf ("> ");
        int i;
        // This loop goes through all the process and checks deletes all the processes that are not alive at the moment
        for (i = 0; i<jobs_len;i++)
        {
            if (waitpid(jobs_list[i].pid,&status, WNOHANG))
            {
                int j;
                for ( j = i; j<jobs_len-1; j++)
                {
                    jobs_list[j] = jobs_list[j+1];
                }
                jobs_len--;
                i--;
            }
        }

        memset(buffer,0,MAX); // We reset the buffer for a new input
        fgets(buffer, MAX, stdin); // We get a new input to the buffer

        if (buffer[0] ==  '\n')
            continue;

        /* We check if there is an '&' at the end of the command and if there is we delete the '&'
        and set the is_background flag to be 1*/
        if (buffer[strlen(buffer) - 2] == '&')
        {
            buffer[strlen(buffer) - 2] = '\n';
            buffer[strlen(buffer) - 1] = '\0';
            is_background = 1;
        }

        // We copy and parse the input into a command frmat using strtok function
        strcpy(temp, buffer);
        strtok(temp, "\n");
        char* command = strtok(temp, " ");
        if ((strcmp(command, "exit") != 0) && strcmp(command, "cd") != 0 && strcmp(command, "jobs") != 0) {
            pid = fork();

            if (pid == -1)
            {
                printf("Error in making a child");
            }

                // If we are at the son process we execute the command and exit
            else if (pid == 0)
            {
                exec(command);
                exit(0);
            }

                // If we are at the father process we insert the son process id and parameters into the job list
            else
            {
                printf("%d\n", pid);
                jobs_list[jobs_len].pid = pid;
                strcpy(jobs_list[jobs_len].parms, buffer);
                jobs_len++;
            }

            // If the is_background flag is off we wait for the son process to be finish before we continue to the next input
            if (!(is_background))
                waitpid(pid,&status,0);
        }

            // If the input was exit we kill all the running process in the jobs list and exit the program
        else if (strcmp(command, "exit") == 0)
        {
            int i;
            printf("%d\n", getpid());
            for (i = 0; i<jobs_len; i++)
                kill(jobs_list[i].pid,SIGKILL);
            exit(0);
        }

            // If the input was cd we will change the directory we are at according to the bash instructions
        else if (strcmp(command, "cd") == 0)
        {
            getcwd(curr_dir, 100);
            char s[100];
            printf("%d\n", getpid());
            command = strtok(NULL, " ");

            // If the directory is not defined or it is ~ then we change the directory to be home
            if (command == NULL || strcmp(command,"~") == 0)
            {
                chdir("/home");
            }

            // If we see - then we go the previous directory we have been at which is saved in the prev_dir variable
            else if (strcmp(command,"-") == 0)
            {
                if (strcmp(prev_dir,"undefined")!=0)
                    chdir(prev_dir);
                else
                {
                    printf("OLDPWD is not set\n");
                }
            }

            // If the path is regular we will change the directory according to it's value while paying attention to "
            else
            {
                int i;
                for (i = 0; i < strlen(buffer); i++)
                {
                    if (buffer[i] == '"')
                    {
                        int j;
                        for (j = i; j < strlen(buffer); j++)
                        {
                            buffer[j] = buffer [j + 1];
                        }
                        quote_mark = !quote_mark;
                    }

                    else if	(!quote_mark && buffer[i] == ' ')
                    {
                        buffer[i] = '\n';
                    }
                }

                command = strtok(buffer, "\n");
                command = strtok(NULL, "\n");
                chdir(command);
            }

            //printf("%s\n", getcwd(s, 100));
            memset(prev_dir,0,MAX);
            strcpy(prev_dir,curr_dir); // We set the prev_dir will be the curr_dir
        }

            // If the user typed jobs we will print out all the running process which are held on the jobs_list
        else if (strcmp(command, "jobs") == 0)
        {
            //printf("%d\n", getpid());
            for (i = 0; i<jobs_len;i++)
            {
                if (waitpid(jobs_list[i].pid,&status, WNOHANG))
                {
                    int j;
                    for ( j = i; j<jobs_len; j++)
                    {
                        jobs_list[j] = jobs_list[j+1];
                    }
                    jobs_len--;
                    i--;
                }
            }
            for (i = 0; i<jobs_len;i++)
            {
                printf("%d %s", jobs_list[i].pid ,jobs_list[i].parms);
            }
        }
    }
}

//This function split a command to it's arguments and execute it using execvp function
int exec(char *command_list)
{
    int i = 0;
    //printf("%d\n", getpid());
    char *argv[MAX];
    // We break to command into argument and insert them into a array
    while (command_list != NULL) {
        argv[i] = command_list;
        command_list = strtok(NULL, " ");
        i++;
    }
    // We execute the command using the execvp and if it fails we print out an error
    if (execvp(argv[0],argv) < 0)
    {
        fprintf(stderr,"Error in system call");
        printf ("\n");
        exit(0);
    }
}