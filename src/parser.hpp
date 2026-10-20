#pragma once

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Grammar:
//   line     := [pipeline] ('&' [pipeline])*
//   pipeline := command ('|' command)*
//   command  := WORD+ ['<' WORD] ['>' WORD]
//
// WORD is a run of characters other than blanks and the operators & | < >.
// A backslash makes the next character part of the word, whatever it is.
// Every pipeline in a line is optional, so an empty line, a lone '&', a
// trailing '&' and `a & & b` are all valid. Empty pipelines are dropped:
// `&` parses to an empty Line, and `a & & b` to the same Line as `a & b`.

// command := WORD+ ['<' WORD] ['>' WORD]
struct Command {
  std::vector<std::string> argv;  // WORD+: program name, then its arguments
  std::optional<std::string> in;  // '<' WORD, or nullopt if absent
  std::optional<std::string> out; // '>' WORD, or nullopt if absent
  bool operator==(const Command &) const = default;
};

// pipeline := command ('|' command)*
//   e.g. `ls | wc -l`
using Pipeline = std::vector<Command>;

// line := [pipeline] ('&' [pipeline])*
//   e.g. `a | b & c` is 2 pipelines run in parallel
using Line = std::vector<Pipeline>;

std::expected<Line, std::string> parse_args(std::string_view line);
std::string to_string(const Line &line);
