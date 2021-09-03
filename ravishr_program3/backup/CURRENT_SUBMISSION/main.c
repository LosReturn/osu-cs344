
#include <sys/types.h>  //pid_t
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // fork()
#include <fcntl.h>
#include <signal.h>

/*
 *struct to store the pid of process and to check if it is a
 * background (set to true).
 * struct will be removed from array if process is completed.
*/
struct process {
  pid_t PID;
  bool background;
};

// array to hold process structs
struct process childpid_arr[200];

// index used to go through childpid_arr of process structs
int processIndex = 0;

// if '&' is found.
bool isBackground = false;
// flag to indicate if background is allowed
bool backgroundAllowed = true;

char **readLine(char *stdinput) {
  // used to place each argument in the 'commandLine' array
  int index = 0;

  /*
   * https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
   * referenced to figure out how to allocate memory for an array of 'strings'
   * will hold the command, args from the commandLine.
  */
  char **commandLine;
  char *saveptr;

  // allocates space for at least 512 arguments
  commandLine = calloc(513, sizeof(char));
  // gets the first argument (will be the command)
  char *token = strtok_r(stdinput, " \n", &saveptr);

  // Loops until no more args left in the line
  while (token != NULL) {
    commandLine[index] = calloc(strlen(token + 1), sizeof(char));
    strcpy(commandLine[index], token);
    index++;
    token = strtok_r(NULL, " \n", &saveptr);
  }

  // Loop through to check for '&'. If it is found, set isBackground to true
  // and remove the '&' from the commandLine array.
  for (int i = 0; commandLine[i] != NULL; i++) {
    if(strcmp(commandLine[i], "&") == 0 && commandLine[i+1] == NULL) {
      commandLine[i] = '\0';
      isBackground = true;
    }
  }
  return commandLine;
}

/*
 * cd - built in command
 * Takes in commandLine array and if directory to change into is not
 * provided, defaults to HOME directory.
 * Otherwise, changes directory to 'arg' provided or prints error message.
*/
int cd_builtin(char **commandLine) {
  char cwd[256];
  if (commandLine[1] == NULL) {
    chdir(getenv("HOME"));
    getcwd(cwd, sizeof(cwd));
  }
  else {
    if (chdir(commandLine[1]) == 0) {
       getcwd(cwd, sizeof(cwd));
    }
    else {
       perror("change directory failed.");
     }
  }
  // returning 1 will allow it to keep looping in main()
  return 1;
}

/*
 * check background process status.
 * if background process is done print out statement and exit value
 * if background process was terminated (kill) print out terminated value
*/
void checkBackgroundProcess(pid_t pid, int index) {
  int status;
  pid_t pid_return = waitpid(pid, &status, WNOHANG);
  if (pid_return > 0) {
    // job is done, so remove process struct from array
    childpid_arr[index].PID = 0;
    childpid_arr[index].background = false;
    processIndex--;
    printf("background pid %d is done: ", pid);
    fflush(stdout);

    // https://www.geeksforgeeks.org/wait-system-call-c/
    // referenced to check how background child process status can be checked.
    if(WIFEXITED(status)) {
      printf("exit value %d\n", WEXITSTATUS(status));
      fflush(stdout);
    }
    else if(WIFSIGNALED(status)) {
      printf("terminated by signal %d\n", WTERMSIG(status));
      fflush(stdout);
    }

  }
}

/*
 * Exploration: Signal Handling API
 * reference code taken from:
 * Example: Custom Handlers for SIGINT, SIGUSR2, and Ignoring SIGTERM, etc.
*/
void handle_SIGTSTP(int signo) {
  if(signo == SIGTSTP) {
    if (backgroundAllowed) {
      backgroundAllowed = false;
      char* message = "\nEntering foreground-only mode (& is now ignored)\n";
      write(STDOUT_FILENO, message, 50);
      fflush(stdout);
    }
    else {
      backgroundAllowed = true;
      char* message = "\nExiting foreground-only mode\n";
      write(STDOUT_FILENO, message, 30);
      fflush(stdout);
    }
  }
}

int main(int argc, char const *argv[]) {
  char **commandLine;
  char *line = NULL;
  size_t len = 0;
  int returnVal;

  /*
   * Ignoring ctrl C
   * Exploration: Signal Handling API
   * reference code taken from
   * Example: Custom Handlers for SIGINT, SIGUSR2, and Ignoring SIGTERM, etc.
  */
  struct sigaction SIGINT_ignore = {0};
  // ignore Ctrl C
  SIGINT_ignore.sa_handler = SIG_IGN;
  sigfillset(&SIGINT_ignore.sa_mask);
  SIGINT_ignore.sa_flags = 0;
  // register the ignore_action as the handler for SIGINT
  // that way this signal will be ignored
  sigaction(SIGINT, &SIGINT_ignore, NULL);

  /*
   * ctrl z handler
   * if ctrl z is entered need to either allow only foreground
   * process or not allow foreground process based on last ctrl z
  */
  struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = handle_SIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  // in exploration Use SA _RESTART Flag in the Signal Handler
	SIGTSTP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  // status of background and forground process
  int fgstatus;
  int bgstatus;

  pid_t shellPid = getppid();

  do {
    printf(": ");
    fflush(stdout);
    /*
     * https://dev-notes.eu/2019/07/Get-a-line-from-stdin-in-C/
     * referenced to see how to get a line from stdin.
     * also referenced exploration: Check for an error and clear it using clearerr
    */
    getline(&line, &len, stdin);
    commandLine = readLine(line);

    // check to see first instance you find $$ because
    // if it is just $ no need to change anything
    for(int i = 0; commandLine[i] != NULL; i++) {
      if((strstr(commandLine[i], "$$")) != NULL) {
        commandLine[i] = realloc(commandLine[i], 6);

        for (int x = 0; commandLine[i][x] != '\0'; x++) {
          if(commandLine[i][x] == '$' && commandLine[i][x+1] == '$') {
            commandLine[i][x] = '\0';
          }
        }
        sprintf(commandLine[i], "%s%d", commandLine[i], shellPid);
      }
    }
    // empty line or #comment
    if (commandLine[0] == NULL || commandLine[0][0] == '#') {
      returnVal = 1;
    }
    // cd command
    else if (strcmp(commandLine[0], "cd") == 0) {
      returnVal = cd_builtin(commandLine);
    }
    // exit command
    else if (strcmp(commandLine[0], "exit") == 0) {
      // loop through entire process array and kill any open process
      for (int i = 0; childpid_arr[i].PID != 0; i++) {
        kill(childpid_arr[i]. PID, SIGKILL);
      }
      returnVal = 0;
    }
    // status command
    else if (strcmp(commandLine[0], "status") == 0) {
      /*
       * print out most recent exit status message that occured
       * for the foreground process
       * https://www.geeksforgeeks.org/wait-system-call-c/
      */
      if(WIFEXITED(fgstatus)) {
        printf("exit value %d\n", WEXITSTATUS(fgstatus));
        fflush(stdout);
      }
      else if(WIFSIGNALED(fgstatus)) {
        printf("terminated by signal %d\n", WTERMSIG(fgstatus));
        fflush(stdout);
      }
      returnVal = 1;
    }
    /*
     * not a built in command
     * Exploration: Process API - Monitoring Child Processes
     * referenced code from waitpid example
    */
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
        // ignore ctrl z in child process
        SIGTSTP_action.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);
        /*
         * Ctrl C should NOT be ignored for a
         * foreground child process and should be handled
         * by the child. Should immediately terminate this child process.
         * Also if background is not allowed the process will
         * be behaving as a forground process
        */
        if (isBackground == false || !backgroundAllowed) {
          // assign the default action of Ctrl C
	        SIGINT_ignore.sa_handler = SIG_DFL;
          sigaction(SIGINT, &SIGINT_ignore, NULL);
        }

        /*
         * Handle redirection input/output
         * Exploration: Processes and I/O
         * referenced code from 'Example: Redirecting both stdin and stdout"
        */
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
        } // end of for loop for handling redirection

        // if it is a background command and background is allowed
        if (isBackground && backgroundAllowed) {
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
        // add child PID to list
        childpid_arr[processIndex].PID = childpid;
        // set background property to the current PID
        if(isBackground && backgroundAllowed) {
          childpid_arr[processIndex].background = true;
        }
        else {
          childpid_arr[processIndex].background = false;
        }

        if(isBackground && backgroundAllowed) {
          printf("background pid is %d\n", childpid_arr[processIndex].PID);
          fflush(stdout);
          pid_t childpid_return = waitpid(childpid, &bgstatus, WNOHANG);
        }
        else {
          sigset_t set;
          sigaddset(&set, SIGTSTP);
          sigprocmask(SIG_BLOCK, &set, NULL);
          pid_t childpid_return = waitpid(childpid, &fgstatus, 0);
          sigprocmask(SIG_UNBLOCK, &set, NULL);

          // if foreground process is done, remove it from array
          if(childpid_return > 0) {
            childpid_arr[processIndex].PID = 0;
            processIndex--;
          }

          // parent must print out a terminated (ctrl C)
          // foreground child process
          if(WIFSIGNALED(fgstatus)) {
            printf("childpid_return=%d ", childpid_return);
            fflush(stdout);
            printf("terminated by signal %d\n", WTERMSIG(fgstatus));
            fflush(stdout);
          }
        }
      }
      returnVal = 1;
      //increment index for the PID array
      processIndex++;
    } // end of else statement (if command is not built in)

    // check background process
    for (int i = 0; childpid_arr[i].PID != 0; i++) {
      if(childpid_arr[i].background == true) {
        checkBackgroundProcess(childpid_arr[i].PID, i);
      }
    }
    // clear buffer for next input
    fflush(stdin);
    // reset the isBackground for next user input
    isBackground = false;
  } while (returnVal == 1); // end of do while loop

  return EXIT_SUCCESS;
}
