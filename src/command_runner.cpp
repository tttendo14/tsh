#include "command_runner.hpp"

#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void run_command(const std::vector<std::string>& args) {
  if (args.empty()) {
    return;
  }

  std::vector<char*> c_args;
  c_args.reserve(args.size() + 1);

  for (const std::string& arg : args) {
    // execv does not modify its argument strings, despite its char* API.
    c_args.push_back(const_cast<char*>(arg.c_str()));
  }
  c_args.push_back(nullptr);

  const pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "Fork failed\n";
    std::exit(1);
  }

  if (pid == 0) {
    execv(c_args[0], c_args.data());
    std::cerr << "execv() failed\n";
    _exit(1);
  }

  if (waitpid(pid, nullptr, 0) == -1) {
    std::cerr << "waitpid() failed\n";
  }
}
