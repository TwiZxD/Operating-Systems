/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"
#include <syscall.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PATH_MAX 50
#define BUFFER_SIZE 25
#define READ_END 0
#define WRITE_END 1
#define STDIN_FILENO 0   /* Standard input. */
#define STDOUT_FILENO 1  /* Standard output. */
#define STDERR_FILENO 2  /* Standard error output. */

/*
 * Function declarations
 */

int ExecuteCommand(Command, int);
void PrintCurrentPath();
void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void SendSignal(int);
void ChildExit();

/* When non-zero, this global means the user is done using this program. */
int done = 0;
int currentPid;
int backPro = 0;

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  int n;

  //redirect the signal received from keyboard to shell
  signal(SIGINT, SendSignal);
  signal(SIGTSTP, SendSignal);

  while (!done) {
    
    signal(SIGCHLD, ChildExit);
    char *line;
    PrintCurrentPath();
    line = readline("> ");

    if (!line) {
      /* Encountered EOF at top level */
      done = 1;
    }
    else {
      /* 
       * Remove leading and trailing whitespace from the line
       * Then, if there is anything left, add it to the history list
       * and execute it.
       */
      
      stripwhite(line);

      if(*line) {
        add_history(line);
        /* execute it */
        n = parse(line, &cmd);
        // PrintCommand(n, &cmd);

        if (!strcmp(*(cmd.pgm->pgmlist), "cd")) {
          char *dir = cmd.pgm->pgmlist[1];
          if (dir) {
            int cdir = chdir(dir);
            if (cdir) {
              printf("No such file or directory\n");
            } 
          } else {
            chdir(getenv("HOME"));
          }
        }
        else if (!strcmp(*(cmd.pgm->pgmlist), "exit")) {
          exit(0);
        } else {
          ExecuteCommand(cmd, -1);
        }
      }
    }
    
    if(line) {
      free(line);
    }
  }
  return 0;
}

/*
 * Name: PrintCurrentPath
 * 
 * Description: Print the current path for the directory
 *
 */
void
PrintCurrentPath ()
{
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s", cwd);
  }
  else {
    perror("getcwd() error");
  }
}

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (isspace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && isspace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}

/*
 * Name: ExecuteCommand
 *
 * Description: execute the command 
 */
int
ExecuteCommand (Command cmd, int parent)
{
  Pgm *p = cmd.pgm;
  cmd.pgm = p->next;
  p->next = NULL;
  FILE *input;
  FILE *output;
  pid_t pid;
  int status;
  int fd[2];

  pid = fork();

  if (pid<0) {
    //error forking
    perror("Fork Failed");
  }
  else if (pid == 0) {
    //child process

    if (cmd.bakground) {
      //background process
      setpgid(0, 0);
    }

    if (cmd.rstdin) {
      //read input file
      input = fopen(cmd.rstdin, "r");
      dup2(fileno(input), READ_END);
      fclose(input);
    }

    if (cmd.rstdout && parent == -1) {
      //write output file
      output = fopen(cmd.rstdout, "w");
      dup2(fileno(output), WRITE_END);
      fclose(output);
    }

    if (!cmd.pgm) {
      //execute single command

      if (parent != -1) {
        //write to the pipe
        dup2(parent, STDOUT_FILENO);
        close(parent);
      } 

      if(execvp(*p->pgmlist, p->pgmlist) < 0) {
        printf("Can't execute the command at last..\n");
        exit(0);
      } 

    } else {
      //pipe
      //create the pipe
      if (pipe(fd) == -1) {
        fprintf(stderr, "Pipe failed");
        return -1;
      }

      ExecuteCommand(cmd, fd[WRITE_END]);

      //read from pipe
      close(fd[WRITE_END]);
      dup2(fd[READ_END], STDIN_FILENO);
      close(fd[READ_END]);

      if (parent != -1) {
        //write to the pipe
        dup2(parent, STDOUT_FILENO);
        close(fd[WRITE_END]);
      } 

      if(execvp(*p->pgmlist, p->pgmlist) < 0) {
        printf("Can't execute the command..\n");
        exit(0);
      } 
    }


   
  }
  else {
    //parent process
    //parent will wait for the child to complete
    //not background process
    if (!cmd.bakground) {
      currentPid = pid;
      waitpid(pid, &status, 0);
      currentPid = 0;
    } 
    
  }
  return 0;
}

/*
 * Name: SendSignal
 *
 * Descibtion: redirect the signal received from the keyboard
 */
void
SendSignal(int signal)
{
  if(currentPid != 0) {
    kill(currentPid, SIGINT);
    currentPid = 0;
  }
}

/*
 * Name: proExit
 *
 * Descibtion: wait until child process terminate
 */
void
ChildExit()
{
  waitpid(-1, 0, WNOHANG);
}