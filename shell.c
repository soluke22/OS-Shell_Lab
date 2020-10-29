#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128
#define MAX_PATH 4096

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
char cwd[MAX_PATH];
char *dir_ptr;
bool background = false;
extern char **environ;

char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void sig_handler(int sig){
  //signal(SIGINT, sig_handler); 
  printf("Try leaving a different way.\n");
  fflush(stdout);
}

char **inputParser(char* command_line)
{
  char **command = malloc(8 * sizeof(char *));
  char *separator = " ";
  char *args;
  char *antr_ptr;
  int i = 0;
  
  args = strtok(command_line, separator);
  while (args != NULL) {
        command[i] = args;
        antr_ptr= command[i];
        if(strchr(antr_ptr,'$') != NULL){
          memmove(antr_ptr, antr_ptr+1, strlen(antr_ptr));
          command[i] = getenv(antr_ptr);
          //command[i] = antr_ptr;
        }
        i++;
        args = strtok(NULL, separator);
    }
  if(strcmp(command[i-1], "&") == 0){
    background = true;
  }
  
  command[i] = NULL;
  return command;  
}


int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
    char *cl_ptr = command_line;
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];
    char **command;
      
    
    while (true) {
      
       
        do{ 
            signal(SIGINT, sig_handler);
            background = false;
            // Print the shell prompt.
            dir_ptr = getcwd(cwd, sizeof(cwd));
            printf("%s%s",dir_ptr, prompt);
            
            fflush(stdout);
            
            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
            //fgets(command_line,MAX_COMMAND_LINE_LEN, stdin);
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
          
          cl_ptr = strsep(&cl_ptr, "\n");
          command = inputParser(cl_ptr);
          
          command[0] = trimwhitespace(command[0]);
          
          if(strcmp(command[0], "exit") == 0){
            exit(0);
          }
          else if(strcmp(command[0], "pwd") == 0){
            dir_ptr = getcwd(cwd, sizeof(cwd));
            printf("%s \n", dir_ptr);
          }
          else if(strcmp(command[0], "echo") == 0){
            int i;
            for(i = 1; i < sizeof(command); i++){
              if(command[i] == NULL){
                break;
              }
              printf("%s ", command[i]);
            }
            printf("\n");
          }
          else if(strcmp(command[0], "env") == 0){
            int i;
            for (i = 0; environ[i] != NULL; i++) 
               printf("\n%s", environ[i]); 
            printf("\n");
          }
          else if(strcmp(command[0], "setenv") == 0){
            const char *str_arr[2];
            char *str = command[1];
            int i =0;
            str_arr[i] = strtok(str, "=");
            while(str_arr[i] != NULL) {
              str_arr[++i] = strtok(NULL,"=");
            }
            setenv(str_arr[0],str_arr[1],1);
          }
          else if(strcmp(command[0], "cd") == 0){
            if(chdir(command[1]) == -1){
              printf("No such directory. \n");
              exit(0);
            }
            
          printf("reached here!\n"); 
          }
          
          else{
            
            pid_t pid;
            pid = fork();
            if (pid == 0) {
              execvp(command[0], command);//make sure the environment variable is set correctly
              printf("reached here!\n");
              sleep(10);
              
              kill(pid, SIGTERM);
             } 
            else {
              if(!background){
                wait(0);
              }
             }
            }
          
        
        }while(command_line[0] == 0x0A);  // while just ENTER pressed
        
      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

    }
    // This should never be reached.
    return -1;
}

