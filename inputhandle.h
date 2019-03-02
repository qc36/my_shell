#ifndef __INPUTHANDLE_H__
#define __INPUTHANDLE_H__
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "myshell.h"

/********
       name:     expand_enval
    discription: expand the variable in command to their value.
      input:     env:the command which have environment variable and need to be expand
      output:    ans:the command after expand all variable it contain
    ********/
std::string expand_enval(std::string env) {
  std::string ans;
  //a vector to store every parts of command which split by "$"
  std::vector<std::string> searchlist;
  //cut off and store the begining part in command which does not contian "$"
  ans.append(env.substr(0, env.find("$")));
  env = env.substr(env.find("$") + 1);
  // when detect "$", split this part and store in searchlist for next detect
  while (env.find("$") != std::string::npos) {
    searchlist.push_back(env.substr(0, env.find("$")));
    env = env.substr(env.find("$") + 1);
  }
  searchlist.push_back(env);
  //a string to store the value of variable which we need to expand
  std::string varvalue;
  //search the searchlist in order to find whether they need to be expand, if so, expand then.otherwise not change them
  for (size_t i = 0; i < searchlist.size(); i++) {
    char * getvalue;
    //a flag to mark whether we find a variable canbe expand or not If it can, the number of this flag represent the length of this variable
    int flag_expand = -1;
    //a string to store the rest part that not belong to the variable name
    std::string expand;
    //search to find the most match part to be expand(the longest part)
    for (size_t j = 0; j < searchlist[i].size(); j++) {
      std::string var_search = searchlist[i].substr(0, j + 1);
      getvalue = getenv(var_search.c_str());
      //if find the variable can be expand, store it and update flag_expand accordingly
      if (getvalue != NULL) {
        varvalue = getvalue;
        flag_expand = j;
      }
    }
    //if there exist variable can be expand, expand it and not change the rest part
    if (!varvalue.empty() || getvalue != NULL) {
      ans.append(varvalue);
      expand = searchlist[i].substr(flag_expand + 1);
      ans.append(expand);
      varvalue.clear();
    }
    //if no such variable exit, not change it
    else {
      ans.append("$");
      ans.append(searchlist[i]);
    }
  }
  return ans;
}

/********
       name:     test_variable
    discription: test the variable name to see whether it is valid
      input:     var: the variable name we need to test
      output:    true if it is valid, false if not
    ********/
bool test_variable(std::string var) {
  //variable name must be a combination of letters (case sensitive), underscores,and numbers so if character rather than these exist in a variable name, it is not valid
  for (std::string::iterator it = var.begin(); it != var.end(); ++it) {
    if ((*it >= 48 && *it <= 57) || (*it >= 65 && *it <= 90) || *it == 95 ||
        (*it >= 97 && *it <= 122)) {
    }
    else {
      return false;
    }
  }
  return true;
}

/********
       name:     getchar_noeof
    discription: check whether it is a EOF everytime read from stdin by getchar
      input:     no input
      output:    no output
    ********/
char getchar_noeof() {
  char inputchar = getchar();
  //if the character get from stdin is EOF, myShell should exit
  if (inputchar == EOF) {
    perror("exit");
    exit(EXIT_FAILURE);
  }
  return inputchar;
}

/********
       name:     get_input
    discription: get input from stdin
      input:     no input
      output:    initial_command: a member of class mushell, which contian the initial command
    ********/
std::vector<std::string> get_input(std::string & initial_input) {
  std::vector<std::string> initial_command;
  // a string to store what we get from stdin
  std::string input;
  // a string to store the input after it is expand
  std::string afterinput;
  char inputchar = getchar_noeof();
  //ignore the whitespace in the begining of input
  while (inputchar == ' ') {
    inputchar = getchar_noeof();
  }
  //store all input untill enter is encountered
  while (inputchar != '\n') {
    input.push_back(inputchar);
    inputchar = getchar_noeof();
  }
  //if received nothing but whitespace before enter, just return the empty command
  if (input.empty()) {
    return initial_command;
  }
  //expand input if "$" is found
  if (input.find("$") != std::string::npos) {
    afterinput = expand_enval(input);
  }
  else {
    afterinput = input;
  }
  //split input into two part for set,inc,cd,export command to store in initial_command
  if (afterinput.find(' ') != std::string::npos) {
    initial_command.push_back(afterinput.substr(0, afterinput.find(' ')));
    initial_command.push_back(afterinput.substr(afterinput.find(' ') + 1));
    //ignore the whitespace in begining of command
    while (*initial_command[1].begin() == ' ') {
      initial_command[1].erase(initial_command[1].begin());
    }
    //if the command is "set",not ignore the whitespace in the end,otherwise just ignore it
    if (initial_command[0] != "set") {
      while (initial_command[1].back() == ' ') {
        initial_command[1].pop_back();
      }
    }
  }
  else {
    initial_command.push_back(afterinput);
  }
  //if command is not set,inc,cd,export  just store all the input
  /* if (initial_command[0] != "set" && initial_command[0] != "inc" && initial_command[0] != "cd" &&
      initial_command[0] != "export") {*/
  initial_input = afterinput;
  //ignore the whitespace in the end
  while (initial_input.back() == ' ') {
    initial_input.pop_back();
  }
  //}
  return initial_command;
}

/********
       name:     set_handle
    discription: deal with the set command
      input:     initial_command: a member of class mushell, which contian the initial command
      output:    true if format of set command is right, otherwise return false
    ********/
bool set_handle(std::vector<std::string> & initial_command) {
  std::string var;
  std::string value;
  //if command not contian the variable name we need to set
  if (initial_command.size() == 1 || initial_command[1].find(' ') == std::string::npos) {
    std::cout << "too few arguments!!" << std::endl;
    return false;
  }
  else {
    //split the value and variable part from initial_command and store them
    std::string value = initial_command[1].substr(initial_command[1].find(' ') + 1);
    initial_command[1] = initial_command[1].substr(0, initial_command[1].find(' '));
    initial_command.push_back(value);
  }
  //if variable name not valid
  if (!test_variable(initial_command[1])) {
    std::cout << "invalid variable name!" << std::endl;
    return false;
  }

  return true;
}
/********
       name:     other_comm_handle
    discription: deal with cd,export,inc command
      input:     initial_command: a member of class mushell, which contian the initial command
      output:    true if format of command is right, otherwise return false
    ********/
bool other_comm_handle(std::vector<std::string> & initial_command) {
  //if command not contian argument
  if (initial_command.size() < 2) {
    std::cout << "too few arguments" << std::endl;
    return true;
  }
  //ignore the whitespace in the begining and end
  while (initial_command[1].front() == ' ') {
    initial_command[1].erase(initial_command[1].begin());
  }
  while (initial_command[1].back() == ' ') {
    initial_command[1].pop_back();
  }
  //if whitespace exist in the middle of command, there are more than one argument exist,which is too much
  if (initial_command[1].find(' ') != std::string::npos) {
    std::cout << "too much arguments" << std::endl;
    return true;
  }
  return false;
}
/********
       name:     inputhandle
    discription: deal with the input read from stdin and translate to command then execute it
      input:     Myshell:an objeect of the class myshell
      output:    no output
    ********/
void inputhandle(myshell & myShell) {
  std::string input;
  //get the current work directory and print promt before everytime receive input
  char * PWD = get_current_dir_name();
  std::cout << "myShell:" << PWD << "$";
  free(PWD);
  //get input from stdin and then store it
  std::vector<std::string> initial_command = get_input(myShell.initial_input);
  //if input is empty, just return and ignore rest of this funtion to read the next input
  if (initial_command.empty() == true) {
    return;
  }
  //if command is exit
  if ((myShell.initial_input == "exit") || feof(stdin)) {
    initial_command.clear();
    std::vector<std::string>().swap(initial_command);
    perror("exit");
    exit(EXIT_FAILURE);
  }
  //if command is cd
  if (initial_command[0] == "cd") {
    if (other_comm_handle(initial_command) == true) {
      return;
    }
    //change current work directory
    myShell.change_path(initial_command[1]);
  }
  //if command is set
  else if (initial_command[0] == "set") {
    if (!set_handle(initial_command)) {
      return;
    }
    //set the variable to environment of myShell
    setenv((char *)initial_command[1].c_str(), (char *)initial_command[2].c_str(), 1);
    //update the pathlist that myShell search for program
    myShell.update_pathlist();
  }
  //if command is export
  else if (initial_command[0] == "export") {
    if (other_comm_handle(initial_command) == true) {
      return;
    }
    //update the envp_export which need to pass into child program
    myShell.update_envp_export(initial_command[1]);
  }
  //if command is inc
  else if (initial_command[0] == "inc") {
    if (other_comm_handle(initial_command) == true) {
      return;
    }
    //increment the value of variable and then update in environment of myShell
    std::pair<std::string, std::string> varvalue = myShell.inc_envir(initial_command[1]);
    setenv((char *)varvalue.first.c_str(), (char *)varvalue.second.c_str(), 1);
    myShell.update_pathlist();
  }
  else {
    initial_command.clear();
    std::vector<std::string>().swap(initial_command);
    //execute the child progarm
    myShell.run_command(myShell.initial_input);
  }
}

#endif
