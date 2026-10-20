#include "parser.hpp"

#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {
enum class TokenType { Word, Lt, Gt, Pipe, Amp };
struct Token {
  TokenType type;
  std::string text;
  Token(TokenType _type, std::string _text)
      : type(_type), text(std::move(_text)) {}
  void print() const {
    if (type == TokenType::Word) {
      std::cerr << "[WORD: " << text << "]";
    } else if (type == TokenType::Lt) {
      std::cerr << "[<]";
    } else if (type == TokenType::Gt) {
      std::cerr << "[>]";
    } else if (type == TokenType::Pipe) {
      std::cerr << "[|]";
    } else if (type == TokenType::Amp) {
      std::cerr << "[&]";
    }
  }
};

bool is_blank(char c) { return c == ' ' || c == '\t'; }

// The token type of a single-character operator, or nullopt if c is not one.
std::optional<TokenType> operator_type(char c) {
  switch (c) {
  case '&':
    return TokenType::Amp;
  case '|':
    return TokenType::Pipe;
  case '<':
    return TokenType::Lt;
  case '>':
    return TokenType::Gt;
  default:
    return std::nullopt;
  }
}

std::expected<std::vector<Token>, std::string>
tokenizer(std::string_view line) {
  std::vector<Token> args;
  std::size_t pos = 0;
  while (pos < line.size()) {
    while (pos < line.size() && is_blank(line[pos])) {
      ++pos;
    }
    if (pos == line.size()) {
      break;
    }

    if (const auto op = operator_type(line[pos])) {
      args.emplace_back(*op, "");
      ++pos;
      continue;
    }

    std::string arg;
    while (pos < line.size() && !is_blank(line[pos]) &&
           !operator_type(line[pos])) {
      // A backslash makes the next character literal, whatever it is.
      if (line[pos] == '\\') {
        if (pos + 1 == line.size()) {
          return std::unexpected<std::string>(
              "tokenizer: a valid line cannot end with a '\\'");
        }
        arg += line[pos + 1];
        pos += 2;
      } else {
        arg += line[pos++];
      }
    }
    args.emplace_back(TokenType::Word, std::move(arg));
  }
  return args;
}

class Parser {
public:
  explicit Parser(std::vector<Token> &&tokens_) : tokens(std::move(tokens_)) {}

private:
  std::vector<Token> tokens;
};

} // namespace

// std::string to_string(const Line &line) {}

std::expected<Line, std::string> parse_args(std::string_view line) {
  auto tokens = tokenizer(line);
  if (!tokens.has_value())
    return std::unexpected(tokens.error());
  Parser parser(std::move(*tokens));
  return std::unexpected<std::string>("placeholder");
}

// int main() {
//   std::string str;
//   std::getline(std::cin, str);
//   tokenizer(str);
//   return 0;
// }
