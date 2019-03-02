#ifndef __MYSHELL_H__
#define __MYSHELL_H__
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
class myshell
{
 private:
  //class Command is a private member of class myshell
  //all of its members are public so that class myshell can ues them
  class Command
  {
   public:
    //the name of program in the command which needed to be execute
    std::string prog_name;
    //the arguments that need to pass in the program we execute
    std::vector<std::string> argument;
    Command() {}
    /********
       name:     path_found
    discription: search pathlist of myshell or directory specify in command to find the specified program we need to execute
      input:     command_noargu:the command name which contian path and program name but not contain arguments
                 pathlist: the member of myshell, which gives the value of environment variable PATH in myshell 
      output:    prog_name:the name of program in the command which needed to be execute
    ********/
    std::string path_found(std::string & command_noargu, std::vector<std::string> & pathlist) {
      std::string prog_name;
      //if command name does not have a '/' in it,search each directory in the pathlist of shell in order for the specified program
      if (command_noargu.find('/') == std::string::npos) {
        prog_name = dir_search(pathlist, command_noargu);
      }
      //if the command name contain a './' in front,it is relative path so no need to search directory, only search directory soecify in command
      //if the command name contain a '/' in it, it is absolute path so only search the directory specify in command
      else {
        //if (command_noargu.find("./") == 0) {
        std::string onedir = command_noargu.substr(0, command_noargu.find_last_of('/') + 1);
        std::cout << onedir << std::endl;
        std::vector<std::string> path = {onedir};
        prog_name = command_noargu.substr(command_noargu.find_last_of('/') + 1);
        std::cout << prog_name << std::endl;
        prog_name = dir_search(path, prog_name);
      }
      //if the command name contain a '/' in it, only search the directory specify in comm
      //return the progname we get
      return prog_name;
    }
    /********
       name:     dir_search
    discription: search a list of directory to find the specified file named command_noargu 
      input:     command_noargu:the command name which contian program name but not contain arguments
                 dir: a vector whihch contian a list of directory which need to search
      output:    file: the absolute name of file we found in the directory(contian the directory it belong)
    ********/
    std::string dir_search(std::vector<std::string> & dir, std::string & command_noargu) {
      DIR * curr_dir;
      struct dirent * entry;
      //search all directories in dir in order
      for (size_t i = 0; i < dir.size(); i++) {
        //if directory cannot open
        if ((curr_dir = opendir(dir[i].c_str())) == 0) {
          std::cout << "cannot open directory!" << std::endl;
          closedir(curr_dir);
          dir.clear();
          std::vector<std::string>().swap(dir);
          command_noargu.clear();
          std::string().swap(command_noargu);
          exit(EXIT_FAILURE);
        }
        else {
          std::string file;
          //search all files in this directory in order to find the file named command_noargu
          while ((entry = readdir(curr_dir)) != NULL) {
            std::string filename(entry->d_name);
            if ((entry->d_type == DT_REG) || (entry->d_type == DT_LNK)) {
              if (filename == command_noargu) {
                //get the absolute name of this file and return it
                std::string dir_curr = dir[i];
                file = dir_curr + "/" + filename;
                closedir(curr_dir);
                return file;
              }
            }
          }
        }
        closedir(curr_dir);
      }
      //if command_noargu can not be found, the return filename is empty
      std::cout << "Command " + command_noargu + " not found" << std::endl;  //not find;
      return "";
    }
    /********
       name:     argumentlist
    discription: get argument list which is needed to pass into child program form initial command which we get from command line
      input:     initial_argu: a string  whih contian the initial formant of arguments we get from command line
      output:    argulist: a vector which contian argument list we need to pass into child program
    ********/
    std::vector<std::string> argumentlist(std::string & initial_argu) {
      std::vector<std::string> argulist;
      //a string to store the arguments temporarily
      std::string argu_temp;
      //search initial_argu in order to put every argument into argulist
      for (std::string::iterator it = initial_argu.begin(); it != initial_argu.end(); ++it) {
        if (*it == ' ') {
          //if there is a '\' before whitespace, then a whilespace escape
          if (it != initial_argu.begin() && *(it - 1) == '\\') {
            argu_temp.back() = ' ';
          }
          else {
            //if there is a whitlespace without '\' before it and argu_temp is not empty, this whitespace is used to seperate
            //so push argu_temp into argulist and clear it to store next argument
            //if argu_temp is empty, just ignore this whitespace
            if (!argu_temp.empty()) {
              argulist.push_back(argu_temp);
              argu_temp.clear();
            }
          }
        }
        //if current character is not whitespace, push it into argu_temp
        else {
          argu_temp.push_back(*it);
        }
      }
      //push the last argument intp argulist
      if (!argu_temp.empty()) {
        argulist.push_back(argu_temp);
      }
      return argulist;
    }
  };
  Command command;
  //a vector to store environment variable which is need to pass to child program, it is private member of myshell
  std::vector<std::string> envp_export;

 public:
  //a vector to store path from environment variable PATH so that we can search and find the specified program we need to execute
  std::vector<std::string> ECE551PATH;
  //a vector to store initial_command get from stdin
  std::string initial_input;
  std::vector<std::string> initial_programs;
  //constructor of myshell
  myshell() : command() { update_pathlist(); };
  //a method which is use to update the pathlist when current work path changed
  void update_pathlist();
  //a method to run child program we need with according arguments and environment variable
  void run_command(std::string & initial_input);
  //a method to execute the inc command of environment variable
  std::pair<std::string, std::string> inc_envir(std::string & var);
  //a method to update environment of myshell, which is use for set command
  void update_envir(std::string & var, std::string & value);
  //a method to update the envp_export, which is environment variable needed to pass into child program
  void update_envp_export(std::string & var);
  //a method to deal with initial command (get from stdin) and turn it into prog_name and argumentlist of corresponding command
  void update_command(std::string & initial_proargu);
  void fork_child(std::string & initial_proargu, int pipe_now[2], int pipe_next[3]);
  void execve_child(std::string & initial_proargu);
  //a method the cut the filename we need to do redirection from initial command
  std::string redir_cut(std::string & initial_argu, std::string redir_type);
  //a method to complete redirct with given redirct type to the filename that give
  bool set_redir(std::string redir_file, int redir_type);
  //a method to deal with redirection of stdin, stdout and error
  void redirect(std::string & initial_input);
  //a method to change current work direcory for cd command
  //splid initial_command into program name and arguments after redirection so that update_command can deal with
  std::vector<std::string> spilid_initial_command(std::string & initial_proargu);
  void change_path(std::string path);
  std::vector<std::string> cut_pipe(std::string & before_pipe);
};
/********
       name:     inc_envir
    discription: Interprets the value of var as a number (base10), and increments it.
      input:     var: a environment variable which is come from set command or environment of myshell
      output:    a pair of var and the new value of it after inc
********/
std::pair<std::string, std::string> myshell::inc_envir(std::string & var) {
  std::pair<std::string, std::string> varvalue;
  varvalue.first = var;
  //a flag to indicate whether old variable of var is a number based on 10 or not. If it is not a number, flag_notnum = 1
  int flag_notnum = 0;
  //the old value of var
  char * newval = getenv(var.c_str());
  std::string newvalue;
  if (newval != NULL) {
    //turn the old value of var to a string
    newvalue = newval;
    //check whether it is a number based on 10 or not
    for (std::string::iterator it = newvalue.begin(); it != newvalue.end(); ++it) {
      if (*it < 48 || *it > 57) {
        flag_notnum = 1;
        break;
      }
    }
    //if old value of var is a number based on 10
    if (flag_notnum == 0) {
      int value_num = atoi(newvalue.c_str()) + 1;
      std::stringstream valuestream;
      valuestream << value_num;
      std::string value;
      valuestream >> value;
      //value+1 become new value of variable
      varvalue.second = value;
    }
    //if old value s not number based on 10, treat it as 0 and plus 1
    else {
      varvalue.second = "1";
    }
  }
  //if var is not set before, treat its value as 0 and plus 1
  else {
    varvalue.second = "1";
  }
  return varvalue;
}

/********
       name:     update_envp_export
    discription: update the envp_export, which is the list of environment variable needed to pass into child program
      input:     var:the value which needed to be export to child program
      output:    no output
********/
void myshell::update_envp_export(std::string & var) {
  //get the value of var
  char * newenvir = getenv(var.c_str());
  //if var is set before or is a variable in environment of myshell
  //add it to envp_export
  if (newenvir != NULL) {
    std::string varvalue = var + "=" + newenvir;
    for (size_t i = 0; i < envp_export.size(); i++) {
      if (var == envp_export[i].substr(0, envp_export[i].find('='))) {
        envp_export[i] = varvalue;
        return;
      }
    }
    envp_export.push_back(varvalue);
  }
  //if it is not set, report error
  else {
    std::cout << "no such variable can be exported or variable name is invalid!" << std::endl;
  }
}

/********
       name:     cut_pipe
    discription: cut the input into different program the we need to execute which split by "|"
      input:     before_pipe:a string of input before split by pipe
      output:    initial_programs: a vector of programs we need to execute
********/
std::vector<std::string> myshell::cut_pipe(std::string & before_pipe) {
  std::vector<std::string> initial_programs;
  while (before_pipe.find("|") != std::string::npos) {
    initial_programs.push_back(before_pipe.substr(0, before_pipe.find("|")));
    before_pipe = before_pipe.substr(before_pipe.find("|") + 1);
  }
  initial_programs.push_back(before_pipe);
  for (size_t i = 0; i < initial_programs.size(); i++) {
    while (initial_programs[i].front() == ' ') {
      initial_programs[1].erase(initial_programs[1].begin());
    }
    while (initial_programs[i].back() == ' ') {
      initial_programs[i].pop_back();
    }
  }
  return initial_programs;
}

/********
       name:     spilid_initial_comand
    discription: spilid command into program name and arguments
      input:     initial_proargu, which is get from stdin and contian name of program we need to run and the argument of it
      output:    no output
********/
std::vector<std::string> myshell::spilid_initial_command(std::string & initial_proargu) {
  std::vector<std::string> initial_command;
  if (initial_proargu.empty()) {
    exit(EXIT_FAILURE);
  }
  while (*initial_proargu.begin() == ' ') {
    initial_proargu.erase(initial_proargu.begin());
  }
  //spelit inital_command to prog_name and arguments two part after redirection
  if (initial_proargu.find(" ") != std::string::npos) {
    initial_command.push_back(initial_proargu.substr(0, initial_proargu.find(" ")));
    initial_command.push_back(initial_proargu.substr(initial_proargu.find(" ") + 1));
  }
  else
    initial_command.push_back(initial_proargu);
  return initial_command;
}

/********
       name:     run_command
    discription: run all the programs in order (the order they cut by "|")
      input:     initial_input, which is cut the redirection part off by method redirect,so that now is a string contian several program name and arguments
     output:     no output
     
********/
void myshell::run_command(std::string & initial_input) {
  //if redirect symbol '<','>','2>' exist in initial_command, do corresponding redirection before execve
  // redirect(initial_input);
  initial_programs = cut_pipe(initial_input);
  int pipe_now[2] = {-1, -1};
  int pipe_prev[2] = {-1, -1};
  for (size_t i = 0; i < initial_programs.size(); i++) {
    redirect(initial_programs[i]);
    std::cout << initial_programs[i] << std::endl;
    int pipeerr = pipe(pipe_now);
    if (pipeerr == -1) {
      std::cout << "pipe error" << std::endl;
    }
    pipe_now[0] = pipe_prev[0];
    pipe_now[1] = pipe_prev[1];
    fork_child(initial_programs[i], pipe_now, pipe_prev);
  }
}

/********
       name:     execve_child
    discription: execve each child program
      input:     initial_proargu: a string  contian the name of program we need to execute and the arguments
     output:     no output

********/
void myshell::execve_child(std::string & initial_proargu) {
  //deal with initial command  and turn it into prog_name and argumentlist of the command we need to run
  update_command(initial_proargu);
  //if no such program exist, exit this child program and return to myshell to receive next command
  if (command.prog_name.empty()) {
    exit(EXIT_FAILURE);
  }

  //change the type of argument list to char **
  char ** argv = new char *[command.argument.size() + 2];
  argv[0] = (char *)command.prog_name.c_str();
  for (size_t i = 0; i < command.argument.size(); i++) {
    argv[i + 1] = (char *)command.argument[i].c_str();
  }
  argv[command.argument.size() + 1] = NULL;

  //change the type of envp_export to char **
  char ** envp = new char *[envp_export.size() + 1];
  for (size_t i = 0; i < envp_export.size(); i++) {
    envp[i] = (char *)envp_export[i].c_str();
  }
  envp[envp_export.size()] = NULL;

  //executes the program pointed to by filename with argument list in argv,environment variable in envp
  execve(argv[0], argv, envp);
  //execve return only when there is an error happend
  perror("execve");
  delete[] envp;
  delete[] argv;
  exit(EXIT_FAILURE);
}

/********
       name:     fork_child
    discription: fork each child program
      input:     initial_proargu: a string  contian the name of program we need to execute and the arguments
     output:     no output

********/
void myshell::fork_child(std::string & initial_proargu, int pipe_now[2], int pipe_prev[2]) {
  //store the pid of child process
  pid_t cpid;
  //store status of child process
  int wstatus;
  //creates  a  new process by duplicating the calling process
  cpid = fork();
  //fork()return -1 when there is an error
  if (cpid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  //fork return 0 when there is child process
  else if (cpid == 0) {
    if (pipe_prev[0] != -1) {
      close(pipe_prev[1]);
      dup2(pipe_prev[0], 0);
    }
    if (pipe_now[0] != -1) {
      close(pipe_now[0]);
      dup2(pipe_now[1], 1);
    }
    //if redirect symbol '<','>','2>' exist in initial_command, do corresponding redirection before execve
    execve_child(initial_proargu);
  }
  //fork()return pid of child process in myshell
  else {
    if (pipe_prev[0] != -1) {
      close(pipe_prev[0]);
      close(pipe_prev[1]);
    }
    pid_t w;
    //myshell is wait for state changes in its child process, and report information about the child whose state has changed.
    w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
    if (w == -1) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    //if child process terminted normally
    if (WIFEXITED(wstatus)) {
      printf("Program exited with status %d\n", WEXITSTATUS(wstatus));
    }
    //if child process terminated by a signal
    else if (WIFSIGNALED(wstatus)) {
      printf("Program was killed by signal %d\n", WTERMSIG(wstatus));
    }
  }
}

/********
       name:     change_path
    discription: change the current work directory, used for "cd" command
      input:     path: the path we wish to change to
      output:    no output
********/
void myshell::change_path(std::string path) {
  if (chdir(path.c_str()) == -1)
    std::cout << "change path failure!" << std::endl;
  //  exit(EXIT_FAILURE);
}

/********
       name:     update_command
    discription: deal with initial command (get from stdin) and turn it into prog_name and argumentlist of corresponding command
      input:     initial_proargu:a string  contian the name of program we need to execute and the arguments
      output:    no output
********/
void myshell::update_command(std::string & initial_proargu) {
  //splid initial_command into two elements: program name and arguments after redirection so that update_command can deal with them
  std::vector<std::string> initial_command = spilid_initial_command(initial_proargu);
  //search the pathlist of myshell to find the program we need to run
  command.prog_name = command.path_found(initial_command[0], ECE551PATH);
  if (initial_command.size() > 1) {
    //get argumentlist which is needed to pass into child program
    command.argument = command.argumentlist(initial_command[1]);
  }
}

/********
       name:     redir_cut
    discription: cut the redirection part from initial arguments and get the name of file we need to redirection
      input:     initial_argu, which is get from stdin and contian any thing rather than the name of program we need to execute
                 redir_type:the sybol of  the redirction type we need to detect(< > 2>)
      output:    redir_file:the name of file that we need to do redirection, if no redirction happpened, return empty string
********/
std::string myshell::redir_cut(std::string & initial_argu, std::string redir_type) {
  std::string redir_file;
  //store the postion of redirection symbol
  size_t pos_redir;
  //stor the number of redireciton
  int num_redir = 0;
  //find position of redirction type, which must be seperate from the rest of initial_argu by whitespace
  while ((initial_argu.find(redir_type) != std::string::npos) &&
         (initial_argu.find(redir_type) == 0 ||
          initial_argu[initial_argu.find(redir_type) - 1] == ' ') &&
         (initial_argu.find(redir_type) == initial_argu.size() - redir_type.size() ||
          initial_argu[initial_argu.find(redir_type) + redir_type.size()] == ' ')) {
    pos_redir = initial_argu.find(redir_type);
    num_redir++;
    //if the file which need redireciton is more than one, just redirection the first one and ignore the rest
    //if redirection symbol is in the end of command, this is a wrong format
    if (pos_redir == initial_argu.size() - redir_type.size()) {
      std::cout << "wrong format of redirection!!" << std::endl;
      exit(EXIT_FAILURE);
    }
    else {
      //store anything after redirection file in initial_argu
      std::string after_redir;
      //store anything before redirection file
      std::string before_redir = initial_argu.substr(0, pos_redir);
      //get the position of redirection file name
      size_t pos_file = pos_redir + redir_type.size();
      //ignore whilespace before the name
      while (initial_argu[pos_file] == ' ') {
        pos_file++;
      }
      while (pos_file < initial_argu.size()) {
        if (initial_argu[pos_file] == ' ') {
          break;
        }
        //only record the name of first redirection file and ignore the rest
        if (num_redir == 1) {
          redir_file.push_back(initial_argu[pos_file]);
        }
        pos_file++;
      }
      if (pos_file < initial_argu.size())
        after_redir = initial_argu.substr(pos_file + 1);
      //delete the redirection part from initial_argu
      initial_argu = before_redir + after_redir;
    }
  }
  return redir_file;
}

/**********
       name:     redirect
    discription: deal with redirection of stdin, stdout and error
      input:     initial_command, which is get from stdin and contian name of program we need to run and the argument of it
      output:    no output
********/
void myshell::redirect(std::string & initial_input) {
  //num of redirection
  std::string initial_argu = initial_input;
  //if there is only one command in initial_command, no need for redirect
  if (initial_input.find(" ") == std::string::npos) {
    return;
  }
  //a vector to store name of file need to redirect, the first element is file stdin need to redirect, second is stdout, third is stderror
  std::vector<std::string> redir_file;
  redir_file.push_back(redir_cut(initial_argu, "<"));
  redir_file.push_back(redir_cut(initial_argu, ">"));
  redir_file.push_back(redir_cut(initial_argu, "2>"));

  //delete the part of redrection from  initial_command
  initial_input = initial_argu;

  for (size_t i = 0; i <= 2; i++) {
    //if the filename is not empty, means this kind of redirection exist and then need to set
    if (!redir_file[i].empty()) {
      //open the file need redirect and store file descriptor of corresponding file
      int file_fd = open(redir_file[i].c_str(), O_RDWR);
      if (file_fd < 0) {
        std::cout << "can not open file!" << std::endl;
        initial_argu.clear();
        std::string().swap(initial_argu);

        exit(EXIT_FAILURE);
      }
      //redirects standard input, file descriptor of stdin is 0, stdout is 1 ,stderr is 2
      int redir_fd = dup2(file_fd, i);
      if (redir_fd < 0) {
        std::cout << "redirect error!" << std::endl;
        initial_argu.clear();
        std::string().swap(initial_argu);
        exit(EXIT_FAILURE);
      }
    }
  }
}
/********
       name:     update_pathlist
    discription: update current pathlist of myshell from environment variable "PATH" and store it in vactor
      input:     no input
      output:    no output
********/
void myshell::update_pathlist() {
  std::vector<std::string> ans;
  //get value of PATH
  char * PATHshell = getenv("PATH");
  std::string pathshell;
  pathshell = PATHshell;
  while (pathshell.find(':') != std::string::npos) {
    ans.push_back(pathshell.substr(0, pathshell.find(':')));
    pathshell = pathshell.substr(pathshell.find(':') + 1);
  }
  ans.push_back(pathshell);
  ECE551PATH = ans;
}

#endif
