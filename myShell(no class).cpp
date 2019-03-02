#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>

//split ECE551path into a vector
std::vector<char *> PATHlist(char * PATH) {
  std::vector<char *> ans;
  char * flag_path = PATH;
  char * curr = PATH;
  while (*curr != '\0') {
    if (*curr == ':') {
      *curr = '\0';
      ans.push_back(flag_path);
      curr++;
      flag_path = curr;
    }
    curr++;
  }
  ans.push_back(flag_path);
  return ans;
}

std::string dir_search(const std::vector<char *> & dir, const std::string & command) {
  DIR * curr_dir;
  struct dirent * entry;
  for (size_t i = 0; i < dir.size(); i++) {
    if ((curr_dir = opendir(dir[i])) == 0) {
      std::cout << "cannot open directory!" << std::endl;
      closedir(curr_dir);
      exit(EXIT_FAILURE);
    }
    else {
      while ((entry = readdir(curr_dir)) != NULL) {
        std::string filename(entry->d_name);
        if (entry->d_type == DT_REG) {
          if (filename == command) {
            std::string dir_curr = dir[i];
            std::string file = dir_curr + "/" + filename;
            closedir(curr_dir);
            std::cout << file << std::endl;
            return file;
          }
        }
      }
    }
  }
  closedir(curr_dir);
  std::cout << "Command " + command + " not found" << std::endl;  //not find;
  exit(EXIT_FAILURE);
  return 0;
}
std::string path_found(std::string command, std::vector<char *> pathlist) {
  std::string execute_file;
  if (command.find('/') == std::string::npos) {
    execute_file = dir_search(pathlist, command);
  }
  else if (command.find("./") == 0) {
    execute_file = command;
  }
  else {
    std::string onedir = command.substr(0, command.find_last_of('/'));
    command = command.substr(command.find_last_of('/') + 1);
    std::vector<char *> path = {(char *)onedir.c_str()};
    execute_file = dir_search(path, command);
    std::cout << execute_file << std::endl;
  }
  return execute_file;
}
int main() {
  char * ECE551PATH = getenv("PATH");
  printf("%s\n", ECE551PATH);
  std::vector<char *> pathlist = PATHlist(ECE551PATH);
  for (size_t i = 0; i < pathlist.size(); i++) {
    std::cout << pathlist[i] << std::endl;
  }
  while (1) {
    std::string command;
    std::cout << "myShell$";
    std::cin >> command;
    if (command == "exit" || feof(stdin)) {
      perror("exit");
      return EXIT_FAILURE;
    }
    pid_t cpid;
    pid_t w;
    cpid = fork();
    int wstatus;
    if (cpid == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else if (cpid == 0) {
      std::cout << "ccc" << std::endl;
      std::string execute_file = path_found(command, pathlist);
      char * newargv[] = {(char *)execute_file.c_str(), NULL};
      char * envp[] = {NULL};
      execve(newargv[0], newargv, envp);
      perror("execve");
      exit(EXIT_FAILURE);
    }
    else {
      w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
      std::cout << "ppp" << std::endl;
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      if (WIFEXITED(wstatus)) {
        printf("Program exited with status %d\n", WEXITSTATUS(wstatus));
      }
      else if (WIFSIGNALED(wstatus)) {
        printf("Program was killed by signal %d\n", WTERMSIG(wstatus));
      }
      else if (WIFSTOPPED(wstatus)) {
        printf("Program was stopped by signal %d\n", WSTOPSIG(wstatus));
      }
    }
  }
  exit(EXIT_SUCCESS);
}
