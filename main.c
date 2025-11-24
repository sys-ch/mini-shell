#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PATH_BUF 4096
#define TYPE_BUF 256
#define MAX_ARGS 64

int main(int argc, char *argv[]) {
  const char *name = "PATH";
  char *env_p = getenv(name);    // pointer to PATH from the environment

  char *command = NULL;          // getline buffer
  size_t cap = 0;                // capacity for getline

  setbuf(stdout, NULL);          // no stdio buffering, print asap
  
  while (1) {
    printf("$ ");

    ssize_t len = getline(&command, &cap, stdin);
    if (len == -1) {
      break;
    }

    if (len > 0 && command[len - 1] == '\n') {
      command[len - 1] = '\0';
      len--;
    }

    // ==== EXIT ====
    if (strcmp(command, "exit") == 0) {
      break;
    }

    // ==== ECHO ====
    if (strncmp(command, "echo", 4) == 0 && (command[4] == '\0' || command[4] == ' ')) {
      // very dumb echo, prints everything after "echo "
      for (int i = 5; i < len; i++) {
        printf("%c", command[i]);
      }
      printf("\n");
      continue;
    }

    // ==== TYPE ====
    if (strncmp(command, "type", 4) == 0 && (command[4] == '\0' || command[4] == ' ')) {
      char type[TYPE_BUF];      // holds the argument to "type"
      char path_c[PATH_BUF];    // local copy of PATH to tokenize
      char conct[PATH_BUF];     // "dir/entry" full path candidate

      char *token;
      bool found = false;
      int k = 0;

      // copy everything after "type " into type[]
      for (int i = 5; i < len && k < TYPE_BUF - 1; i++) {
        type[k++] = command[i];
      }
      type[k] = '\0';

      // check builtins first
      if (strcmp(type, "echo") == 0) {
        printf("%s is a shell builtin\n", type);
        continue;
      } else if (strcmp(type, "exit") == 0) {
        printf("%s is a shell builtin\n", type);
        continue;
      } else if (strcmp(type, "type") == 0) {
        printf("%s is a shell builtin\n", type);
        continue;
      } else {
        // copy PATH into a scratch buffer so strtok can mess with it
        strncpy(path_c, env_p, sizeof(path_c) - 1);
        path_c[sizeof(path_c) - 1] = '\0';

        // walk each directory in PATH
        token = strtok(path_c, ":");
        while (token != NULL) {
          DIR *dir = opendir(token);
          if (dir == NULL) {
            token = strtok(NULL, ":");
            continue;
          }

          struct dirent *entry;
          // scan directory entries
          while ((entry = readdir(dir)) != NULL) {
            // build "dir/filename" into conct
            snprintf(conct, sizeof(conct), "%s/%s", token, entry->d_name);
            if (strcmp(entry->d_name, type) == 0 && access(conct, X_OK) == 0) {
              found = true;
              break;
            }
          }
          closedir(dir);
          if (found) {
            break;
          }
          token = strtok(NULL, ":");
        }
      }

      if (found) {
        printf("%s is %s\n", type, conct);
        continue;
      } else {
        printf("%s: not found\n", type);
        continue;
      }
    }

    // ==== EXECUTE EXTERNAL COMMAND ====
    else {
      char *args[MAX_ARGS];
      int args_n = 0;

      // simple split on spaces/tabs/newlines
      char *token = strtok(command, " \t\n");
      while (token != NULL && args_n < MAX_ARGS - 1) {
        args[args_n++] = token;
        token = strtok(NULL, " \t\n");
      }
      args[args_n] = NULL;  // execvp expects a NULL-terminated array

      // empty line: nothing to do, go back to prompt
      if (args_n == 0) {
        continue;
      }

      char *program = args[0];
      pid_t pid = fork();
      
      if (pid == 0) {
        if (execvp(program, args) == -1) {
          fprintf(stderr, "Error executing '%s': %s\n", program, strerror(errno));
          _exit(1);
        }
      } else if (pid > 0) {
        wait(NULL);
      } else {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
      }
      continue;
    }
  }

  // free getline buffer before leaving
  free(command);
  return 0;
}
