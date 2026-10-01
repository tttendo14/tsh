#include "parser.hpp"

#include <utility>

std::vector<std::string> parse_args(std::string_view line) {
  std::vector<std::string> args;
  std::size_t pos = 0;

  while (pos < line.size()) {
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
      ++pos;
    }

    if (pos == line.size()) {
      break;
    }

    std::string arg;
    while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t') {
      if (line[pos] == '\\' && pos + 1 < line.size() &&
          (line[pos + 1] == ' ' || line[pos + 1] == '\t')) {
        arg += line[pos + 1];
        pos += 2;
      } else {
        arg += line[pos++];
      }
    }

    args.push_back(std::move(arg));
  }

  return args;
}
