#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>
// An escaped space or tab is part of its argument.

struct Command {
  std::vector<std::string> argv;
  std::optional<std::string> in;
  std::optional<std::string> out;
  bool operator==(const Command &) const = default;
};
using Pipeline = std::vector<Command>;
using Line = std::vector<Pipeline>;

Line parse_args(std::string_view line);
