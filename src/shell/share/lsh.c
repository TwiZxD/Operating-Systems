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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"

#include <stdbool.h>
#include <string.h>
#include <syscall.h>


/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
char currentDirectory[50];

int PATH_MAX = 50;
/* When non-zero, this global means the user is done using this program. */
int done = 0;


int createProcessRun(Command cmd) {
	Pgm *pgm = cmd.pgm;
	char **pgmlist = pgm->pgmlist;
	cmd.pgm = pgm->next;

	int status;
	if(fork() != 0) {
		//Parent Code
		waitpid(-1, &status, 0);

	} else {
		//Child Code
		execvp(*pgmlist, pgmlist);
		
		return 0;
	}

	return 0;
}

/*
 * Name: printCurrentWorkingDirectory
 *
 * Description: Display the working directory.
 *
 */

int printCurrentWorkingDirectory() {
	char cwd[PATH_MAX];
	if(getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("%s", cwd);

	} else {
		perror("getcwd() error");
		return 1;
	}
	return 0;
}

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

  while (!done) {

	char *line;
	printCurrentWorkingDirectory();
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
 
 		//Code here
 		if (!strcmp (*(cmd.pgm->pgmlist), "exit")) {
 			exit(0);
 		}

 		if (!strcmp (*(cmd.pgm->pgmlist), "cd")) {

 			if(cmd.pgm->pgmlist[1]) {
 				int dir = chdir(cmd.pgm->pgmlist[1]);
 				if(dir) {
 					printf("No such directory! \n");
 				} 
 			} else {
 				chdir(getenv("HOME"));
 			}
 			
 		} else {
 			//Run Command here
 			createProcessRun(cmd);
 			
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
  //printf("n: " %d ,n );
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
