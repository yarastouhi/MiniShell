#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h> 
#include "parser.h"
#include "utils.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
//beginning of student defined functions----------------------------------------------------------------
int *bgPID = NULL;
char **bgCommands = NULL;
int bgCount = 0;

void redirectIn(struct cmdline *s) {
    char **cmd = s->seq[0];
    int file = open(s->in, O_RDONLY);
    dup2(file, STDIN_FILENO); 
    close(file);
}

void addJob(int PID, char *command) {
    bgCount++;
    bgPID = xrealloc(bgPID, bgCount * sizeof(int));
    bgCommands = xrealloc(bgCommands, bgCount * sizeof(char *));
    bgPID[bgCount - 1] = PID;
    bgCommands[bgCount - 1] = strdup(command);
}

void removeJob() {
    for (int i = 0; i < bgCount; i++) {
        int stat;
        int running = waitpid(bgPID[i], &stat, WNOHANG);
        if (running == bgPID[i]) {
            free(bgCommands[i]);
            for (int j = i; j < bgCount - 1; j++) {
                bgPID[j] = bgPID[j + 1];
                bgCommands[j] = bgCommands[j + 1];
            }
            bgCount--;
            if (bgCount <= 0) {
                bgCount = 0;
                free(bgPID);
                free(bgCommands);
                bgPID = NULL;
                bgCommands = NULL;
            } else {
                bgPID = xrealloc(bgPID, bgCount * sizeof(int));
                bgCommands = xrealloc(bgCommands, bgCount * sizeof(char *));
            }
            i--;
        }
    }
}

void printJob(){
    printf("PID    Command\n");
    for(int i = 0; i < bgCount; i++){
        printf("%d    %s\n",bgPID[i],bgCommands[i]);
        removeJob();
    }
}

void redirectOut(struct cmdline *s) {
    char **cmd = s->seq[0];
    int file = open(s->out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(file, STDOUT_FILENO); 
    close(file); 
}
void execute_command(struct cmdline *l) {
    int i = 0, in_fd = 0;
    int pipefd[2]; 

    while (l->seq[i] != 0) { //checking for multiple pipes
        if (l->seq[i + 1] != 0) { 
            if (pipe(pipefd) == -1) { 
                perror("pipe failed");
                exit(EXIT_FAILURE);
            }
        }

        pid_t pid1 = fork(); 
        if (pid1 < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pid1 == 0) { 
            // Input redirection from the previous command
            if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            // Output redirection to the next command
            if (l->seq[i + 1] != 0) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }

            // Handle input/output redirection for first/last commands
            if (l->out != 0) {
                redirectOut(l);
            }
            
            if (l->in != 0) {
                redirectIn(l); 
            }

            char **cmd = l->seq[i];
            if (execvp(cmd[0], cmd) == -1) {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        } else {
            if (in_fd != 0) {
                close(in_fd);
            }

            if (l->seq[i + 1] != 0) {
                close(pipefd[1]); 
            }

            in_fd = pipefd[0]; 
            i++;

            if (l->bg == 0) { 
                int status;
                waitpid(pid1, &status, 0);
            }
        }
    }
}


void check_background(struct cmdline *l) {
    char **cmd = l->seq[0];
    char *command = cmd[0];
    removeJob();
    if (l->bg == 1) {
        pid_t pid1 = fork();
        addJob(pid1, command);
        if (pid1 == 0) {
            execute_command(l);
            exit(0);
        }
    } else {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            execute_command(l);
            exit(0); 
        } else {
            int status; 
            waitpid(pid2, &status, 0); 
        }
    }
    
}

//end of student defined functions----------------------------------------------------------------

void terminate(char *line) {
    if (line)
        free(line); //release memory allocated to line pointer
    printf("bye\n");
    exit(0);
}

/* Read a line from standard input and put it in a char[] */
char* readline(const char *prompt)
{
    size_t buf_len = 16;
    char *buf = xmalloc(buf_len * sizeof(char));

    printf("%s", prompt);
    if (fgets(buf, buf_len, stdin) == NULL) {
        free(buf);
        return NULL;
    }

    do {
        size_t l = strlen(buf);
        if ((l > 0) && (buf[l-1] == '\n')) {
            l--;
            buf[l] = 0;
            return buf;
        }
        if (buf_len >= (INT_MAX / 2)) memory_error();
        buf_len *= 2;
        buf = xrealloc(buf, buf_len * sizeof(char));
        if (fgets(buf + l, buf_len - l, stdin) == NULL) return buf;
    } while (1);
}

int main(void) {
    while (1) {
        struct cmdline *l;
        char *line = 0;
        char *prompt = "myshell>";

        line = readline(prompt); // Read the input from the user
        if (line == 0 || !strncmp(line, "exit", 4)) {
            terminate(line); // Exit the shell if "exit" is typed
        } else {
            l = parsecmd(&line); // Parse the command

            if (l == 0) {
                terminate(0); // If the input stream is closed, terminate
            } else if (l->err != 0) {
                printf("error: %s\n", l->err); // Handle syntax errors
                continue;
            }

            char **cmd = l->seq[0];
            if (cmd != NULL && strcmp(cmd[0], "jobs") == 0) {
                printJob();
            } else {
                check_background(l); 
            }
        }
    }
}