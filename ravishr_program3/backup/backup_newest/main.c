
#include <sys/types.h>  //pid_t
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // fork()
#include <fcntl.h>


//pid_t childpid_arr[200];

struct process {
  pid_t PID;
  bool background;
};

struct process childpid_arr[200];

int processIndex = 0;

bool isBackground = false;

char **readLine(char *stdinput) {
  // used to place each argument in the 'args' array
  int index = 0;

  // https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
  // referenced to figure out how to allocate memory for an array of 'strings'
  // will hold the command, args from the line
  char **commandLine;
  char *saveptr;

  // allocates space for at least 512 arguments
  commandLine = calloc(513, sizeof(char));
  // gets the first argument (command)
  char *token = strtok_r(stdinput, " \n", &saveptr);

  // Loops until no more args left in the line
  while (token != NULL) {
    commandLine[index] = calloc(strlen(token + 1), sizeof(char));
    strcpy(commandLine[index], token);
    index++;
    token = strtok_r(NULL, " \n", &saveptr);
  }

  // loop through to check for &. If it is set isBackground to true
  // and remove the & from the commandLine array.
  for (int i = 0; commandLine[i] != NULL; i++) {
    if(strcmp(commandLine[i], "&") == 0) {
      commandLine[i] = '\0';
      isBackground = true;
    }
  }

  return commandLine;
}

int cd_builtin(char **commandLine) {
  char cwd[256];
  if (commandLine[1] == NULL) {
    chdir(getenv("HOME"));
    getcwd(cwd, sizeof(cwd));
    printf("cwd=%s\n", cwd);
  }
  else {
    if (chdir(commandLine[1]) == 0) {
       getcwd(cwd, sizeof(cwd));
       printf("cwd=%s\n", cwd);
    }
    else {
       perror("smallsh: chdir error.");
     }
  }
  // returning 1 will allow it to keep looping in main()
  return 1;
}

void checkBackgroundProcess(pid_t pid, int index) {
  int status;
  pid_t pid_return = waitpid(pid, &status, WNOHANG);
  if (pid_return > 0) {
    childpid_arr[index].background = false;
    printf("background pid %d is done: ", pid);

    // https://www.geeksforgeeks.org/wait-system-call-c/
    if(WIFEXITED(status)) {
      printf("exit value %d\n", WEXITSTATUS(status));
    }
    else if(WIFSIGNALED(status)) {
      printf("terminated by signal %d\n", WTERMSIG(status));
    }
    fflush(stdout);
  }
}

int main(int argc, char const *argv[]) {
  char **commandLine; // calloc(2048, sizeof(char))
  char *line = NULL;
  size_t len = 0;
  int returnVal;

  int fgstatus;
  int bgstatus;

  pid_t shellPid = getppid();
  printf("shellPid=%d\n", shellPid);
  do {
    printf(": ");
    // https://dev-notes.eu/2019/07/Get-a-line-from-stdin-in-C/
    // referenced to see how to get a line from stdin.
    getline(&line, &len, stdin);
    commandLine = readLine(line);

    // empty line or #comment
    if (commandLine[0] == NULL || strcmp(commandLine[0], "#") == 0) {
      returnVal = 1;
    }
    else if (strcmp(commandLine[0], "cd") == 0) {
      //printf("in here.\n");
      returnVal = cd_builtin(commandLine);
    }
    else if (strcmp(commandLine[0], "exit") == 0) {

      // loop through entire process array and kill any open process
      for (int i = 0; childpid_arr[i].PID != 0; i++) {
        kill(childpid_arr[i]. PID, SIGKILL);
      }
      returnVal = 0;
    }
    else if (strcmp(commandLine[0], "status") == 0) {

      // print out most recent exit status message that occured
      // for the foreground process
      // https://www.geeksforgeeks.org/wait-system-call-c/
      if(WIFEXITED(fgstatus)) {
        printf("exit value %d\n", WEXITSTATUS(fgstatus));
      }
      else if(WIFSIGNALED(fgstatus)) {
        printf("terminated by signal %d\n", WTERMSIG(fgstatus));
      }
    }
    // Exploration: Process API - Monitoring Child Processes
    // referenced code from waitpid example
    else {
      pid_t childpid;
      bool foundIOredir = false;

      bool foundInputRedirection = false;
      bool foundOutputRedirection = false;

      childpid = fork();

      if (childpid == -1) {
        perror("fork() failed.");
      }
      else if (childpid == 0) {
        //bool isBg = false;
        //char **commandLineCopy;

        // Handle redirection input/output
        // Exploration: Processes and I/O
        // referenced code from 'Example: Redirecting both stdin and stdout"
        for (int i = 1; commandLine[i] != NULL; i++) {

          // if input redirection is specified
          if(strcmp(commandLine[i], "<") == 0) {
            foundIOredir = true;
            foundInputRedirection = true;
            int sourceFD = open(commandLine[i+1], O_RDONLY);
            if(sourceFD == -1) {
              printf("cannot open %s for input\n", commandLine[i+1]);
              fflush(stdout);
              exit(1);
            }
            int resultSource = dup2(sourceFD, 0);
            if(resultSource == -1) {
              printf("dup2() error with %s as input\n", commandLine[i+1]);
              fflush(stdout);
              exit(1);
            }
            close(sourceFD);
          }

          // if output redirection is specified
          if(strcmp(commandLine[i], ">") == 0) {
            foundIOredir = true;
            foundOutputRedirection = true;
            int targetFD = open(commandLine[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(targetFD == -1) {
              printf("cannot open %s for output\n", commandLine[i+1]);
              fflush(stdout);
              exit(1);
            }
            int resultTarget = dup2(targetFD, 1);
            if(resultTarget == -1) {
              printf("dup2() error with %s as output\n", commandLine[i+1]);
              fflush(stdout);
              exit(1);
            }
            close(targetFD);
          }
        } // end of for loop

        // if it is a background command
        if (isBackground) {
          // if input redirection was NOT specified
          // background command should use /dev/nul
          if(foundInputRedirection != true) {
            int bgsourceFD = open("/dev/null", O_RDONLY);
            if(bgsourceFD == -1)
              perror("background source open()");
            int bgresultSource = dup2(bgsourceFD, 0);
            if(bgresultSource == -1)
              perror("background source dup2()");
            close(bgsourceFD);
          }

          // if output redirection was NOT specified
          // background command should use /dev/null
          if(foundOutputRedirection != true) {
            int bgtargetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(bgtargetFD == -1)
              perror("background target open()");
            int bgresultTarget = dup2(bgtargetFD, 1);
            if(bgresultTarget == -1)
              perror("background target dup2()");
            close(bgtargetFD);
          }

        }

        // if no redirection was specified
        if(foundIOredir != true) {
          if (execvp(commandLine[0], commandLine) == -1) {
            printf("%s cannot be executed: No such file or directory\n", commandLine[0]);
            fflush(stdout);
            exit(1);
          }
        }
        // if redirection was specified
        else {
          if(execlp(commandLine[0], commandLine[0], NULL) == -1) {
            printf("%s cannot be executed: No such file or directory\n ", commandLine[0]);
            fflush(stdout);
            exit(1);
          }
        }
      }
      else {

        childpid_arr[processIndex].PID = childpid; // add child PID to list
        printf("childpid_arr[%d]=%d\n", processIndex, childpid_arr[processIndex].PID);

        // set background property to the current PID
        if(isBackground) {
          childpid_arr[processIndex].background = true;
        }
        else {
          childpid_arr[processIndex].background = false;
        }
        printf("childpid_arr[%d]=%s\n", processIndex, childpid_arr[processIndex].background ? "true" : "false");

        if(isBackground == true) {
          // keep this printf statement
          printf("background pid is %d\n", childpid_arr[processIndex].PID);
          fflush(stdout);
          pid_t childpid_return = waitpid(childpid, &bgstatus, WNOHANG);
        }
        else {
          pid_t childpid_return = waitpid(childpid, &fgstatus, 0);

          // if foreground process is done, remove it from array
          if(childpid_return > 0) {
            childpid_arr[processIndex].PID = 0;
            processIndex--;
          }
        }


        // REMOVE BELOW LATER
        if (WIFEXITED(fgstatus)) {
          printf("successful child exit.\n");
        }
        else {
          printf("abnormal child exit.\n");
        }

      }
      returnVal = 1;
    }
    // clear buffer for next input
    fflush(stdin);

    // check background process
    for (int i = 0; childpid_arr[i].PID != 0; i++) {
      if(childpid_arr[i].background == true) {
        checkBackgroundProcess(childpid_arr[i].PID, i);
      }
    }

    // reset the isBackground for next user input
    isBackground = false;
    processIndex++; //increment index for the PID array


  } while (returnVal == 1);


  return EXIT_SUCCESS;
}
