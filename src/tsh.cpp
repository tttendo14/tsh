#include "tsh.hpp"
#include "command_runner.hpp"
#include "parser.hpp"

#include <algorithm>
#include <cerrno>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
// #include <utility>
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

    std::expected<Line, std::string> parsed = parse_args(line);
    if (!parsed) {
      std::cerr << "tsh: " << parsed.error() << std::endl;
      continue;
    }
    if (parsed->empty()) {
      continue;
    }

    // TODO(M5): '|'. Until then, only a single pipeline is run.
    // Line[0]: Pipeline, Pipeline[0]: Command
    //

    bool has_pipe = std::ranges::any_of(
        *parsed, [](const Pipeline &p) -> bool { return p.size() > 1; });

    if (has_pipe) {
      std::cerr << "tsh: '|' are not implemented yet" << std::endl;
      continue;
    }

    bool exit = false;
    for (const auto &pipeline : *parsed) {
      const Command &cmd = pipeline[0];

      CommandExit_t command_rv = run_command(cmd);
      if (command_rv == CommandExit_t::Exit)
        exit = true;
    }
    // parent process
    while (waitpid(-1, nullptr, 0) > 0) {
    }

    if (exit) {
      break;
    }
  }

  return 0;
}
