#include "tsh.hpp"
#include "command_runner.hpp"
#include "parser.hpp"
#include "shell_path.hpp"

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

void print_prompt() { std::cout << "tsh> "; }

int main(int argc, char *argv[]) {

  bool batchMode = (argc != 1);
  // std::cerr << "batchMode = " << batchMode << std::endl;

  std::ifstream ifstream;
  if (argc == 2) {
    ifstream = std::ifstream(argv[1]);
    if (!ifstream.is_open()) {
      std::cerr << "Failed to open batch file " << argv[1] << std::endl;
      exit(1);
    }
  } else if (argc > 2) {
    std::cerr << "Invalid argument" << std::endl;
    exit(1);
  }

  std::istream *p_in = batchMode ? (&ifstream) : (&std::cin);

  while (true) {
    if (!batchMode)
      print_prompt();

    std::string line;
    if (!std::getline(*p_in, line)) {
      break;
    }

    std::vector<std::string> args = parse_args(line);
    if (args.empty()) {
      continue;
    }

    if (args[0] == "exit") {
      if (args.size() != 1) {
        std::cerr << "exit: Invalid argument" << std::endl;
        continue;
      }
      return 0;
    } else if (args[0] == "cd") {
      if (args.size() == 1) {
        const char *home = std::getenv("HOME");
        if (home != nullptr) {
          chdir(home);
        } else {
          std::cerr << "cd: Failed to get $HOME directory" << std::endl;
        }
        continue;
      } else if (args.size() != 2) {
        std::cerr << "cd: Invalid argument" << std::endl;
        continue;
      }
      if (chdir(args[1].c_str()) == -1) {
        std::cerr << "cd: chdir() error, perhaps an invalid directory was given"
                  << std::endl;
        continue;
      }

      continue;
    } else if (args[0] == "path") {
      std::vector<std::string> dirs{args.begin() + 1, args.end()};
      ShellPath::instance().setDirs(std::move(dirs));
      continue;
    }

    run_command(args);
  }

  return 0;
}
