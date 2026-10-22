#include "command_runner.hpp"
#include "shell_path.hpp"

#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

pid_t spawn_child(const Command &command) {
  const std::vector<std::string> &args = command.argv;

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
  } else {
    return pid;
  }
}

Builtin_t run_builtin(const Command &command) {
  const std::vector<std::string> &args = command.argv;
  if (args[0] == "exit") {
    if (args.size() != 1) {
      std::cerr << "exit: Invalid argument" << std::endl;
      return Builtin_t::Handled;
    }
    return Builtin_t::Exit;
  } else if (args[0] == "cd") {
    if (args.size() == 1) {
      const char *home = std::getenv("HOME");
      if (home != nullptr) {
        chdir(home);
      } else {
        std::cerr << "cd: Failed to get $HOME directory" << std::endl;
      }
      return Builtin_t::Handled;
    } else if (args.size() != 2) {
      std::cerr << "cd: Invalid argument" << std::endl;
      return Builtin_t::Handled;
    }
    if (chdir(args[1].c_str()) == -1) {
      std::cerr << "cd: chdir() error, perhaps an invalid directory was given"
                << std::endl;
      return Builtin_t::Handled;
    }
    return Builtin_t::Handled;
  } else if (args[0] == "path") {
    std::vector<std::string> dirs{args.begin() + 1, args.end()};
    ShellPath::instance().setDirs(std::move(dirs));
    return Builtin_t::Handled;
  }
  return Builtin_t::NotABuiltIn;
}

CommandExit_t run_command(const Command &command) {
  if (command.argv.empty()) {
    return CommandExit_t::Continue;
  }

  // const std::vector<std::string> &args = command.argv;
  Builtin_t builtin_rv = run_builtin(command);

  switch (builtin_rv) {
  case Builtin_t::NotABuiltIn:
    break;
  case Builtin_t::Handled:
    return CommandExit_t::Continue;
  case Builtin_t::Exit:
    return CommandExit_t::Exit;
  }

  // pid_t pid = spawn_child(command);
  spawn_child(command);

  return CommandExit_t::Continue;
}
