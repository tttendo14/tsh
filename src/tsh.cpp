#include "tsh.hpp"
#include "command_runner.hpp"
#include "parser.hpp"

#include <iostream>
#include <string>
#include <vector>

void print_prompt() { std::cout << "tsh> "; }

int main() {
  while (true) {
    print_prompt();

    std::string line;
    if (!std::getline(std::cin, line)) {
      break;
    }

    std::vector<std::string> args = parse_args(line);
    if (args.empty()) {
      continue;
    }

    if (args[0] == "exit") {
      return 0;
    }
    if (args[0] == "cd" || args[0] == "path") {
      continue;
    }

    run_command(args);
  }

  return 0;
}
