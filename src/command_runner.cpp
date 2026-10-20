#include "command_runner.hpp"
#include "shell_path.hpp"

#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

void run_command(const Command &command) {

  const std::vector<std::string> &args = command.argv;

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
    if (command.in) {
      int fd_in = open(command.in.value().c_str(), O_RDONLY);
      if (dup2(fd_in, STDIN_FILENO) == -1) {
        perror(command.in.value().c_str());
        _exit(1);
      }
      close(fd_in);
    }
    if (command.out) {
      int fd_out =
          open(command.out.value().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
      // 0644 == (644)_8 == rw-r--r--
      if (dup2(fd_out, STDOUT_FILENO) == -1) {
        perror(command.out.value().c_str());
        _exit(1);
      }
      close(fd_out);
    }
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
