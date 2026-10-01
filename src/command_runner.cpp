#include "command_runner.hpp"
#include "shell_path.hpp"

#include <cstdlib>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void run_command(const std::vector<std::string> &args) {
  if (args.empty()) {
    return;
  }

  std::vector<char *> c_args;
  c_args.reserve(args.size() + 1);

  for (const std::string &arg : args) {
    // execv does not modify its argument strings, despite its char* API.
    c_args.push_back(const_cast<char *>(arg.c_str()));
  }
  c_args.push_back(nullptr);

  const pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "Fork failed\n";
    std::exit(1);
  }

  if (pid == 0) { // child process
    bool containsSlash = false;
    for (size_t i = 0; i < args[0].size(); i++)
      if (args[0][i] == '/')
        containsSlash = true;
    if (containsSlash) {
      execv(args[0].c_str(), c_args.data());
    }
    for (const std::string &dir : ShellPath::instance().getDirs()) {
      const std::string executable = dir + "/" + args[0];
      execv(executable.c_str(), c_args.data());
    }

    std::cerr << "execv() failed\n";
    _exit(1);
  }

  // parent process
  if (waitpid(pid, nullptr, 0) == -1) {
    std::cerr << "waitpid() failed\n";
  }
}
