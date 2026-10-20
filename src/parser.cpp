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
  explicit Parser(std::vector<Token> &&tokens_)
      : tokens(std::move(tokens_)), pos(0) {}
  // NOTE: Line := [Pipeline] ('&' [Pipeline])*
  std::expected<Line, std::string> line() {
    Line res;

    // [Pipeline]: a pipeline always starts with a WORD, so that's the test.
    // An empty slot (e.g. around a lone '&') is skipped, not an error.
    auto optional_pipeline = [&]() -> std::expected<void, std::string> {
      if (!peek(TokenType::Word))
        return {};
      auto p = pipeline();
      if (!p)
        return std::unexpected(p.error());
      res.push_back(std::move(*p));
      return {};
    };

    if (auto r = optional_pipeline(); !r)
      return std::unexpected(r.error());
    while (accept(TokenType::Amp)) {
      if (auto r = optional_pipeline(); !r)
        return std::unexpected(r.error());
    }

    // Every rule stops at the first token it doesn't understand, so leftover
    // tokens (e.g. the `b` in `ls > a b`, or a leading `|`) end up here.
    if (!at_end())
      return std::unexpected<std::string>(
          "Invalid command: unexpected token after position " +
          std::to_string(pos));
    return res;
  }

private:
  bool at_end() const { return pos == tokens.size(); }
  bool peek(TokenType t) const { return !at_end() && tokens[pos].type == t; }
  bool accept(TokenType t) {
    if (!peek(t))
      return false;
    ++pos;
    return true;
  }
  std::optional<std::string> accept_word() {
    if (!peek(TokenType::Word))
      return std::nullopt;
    return std::move(tokens[pos++].text);
  }

  std::expected<Command, std::string> command() {
    Command res;
    while (peek(TokenType::Word)) {
      res.argv.emplace_back(std::move(accept_word().value()));
    }
    if (res.argv.empty()) {
      return std::unexpected<std::string>(
          "Invalid command: command cannot be empty");
    }
    if (peek(TokenType::Lt)) {
      accept(TokenType::Lt);
      if (!peek(TokenType::Word)) {
        return std::unexpected<std::string>(
            "Invalid command: illegal character after <");
      }
      res.in = std::move(accept_word().value());
    }
    if (peek(TokenType::Gt)) {
      accept(TokenType::Gt);
      if (!peek(TokenType::Word)) {
        return std::unexpected<std::string>(
            "Invalid command: illegal character after >");
      }
      res.out = std::move(accept_word().value());
    }
    return res;
  }
  // NOTE: Pipeline := Command ('|' Command)*
  std::expected<Pipeline, std::string> pipeline() {
    Pipeline res;
    do {
      auto cmd = command();
      if (!cmd)
        return std::unexpected(cmd.error());
      res.push_back(std::move(*cmd));
    } while (accept(TokenType::Pipe));
    return res;
  }

  std::vector<Token> tokens;
  size_t pos;
};

} // namespace

namespace {
// Backslash-escape every character the tokenizer would otherwise treat
// specially, so that parse_args(to_string(l)) == l.
std::string escape(const std::string &word) {
  std::string res;
  for (char c : word) {
    if (is_blank(c) || operator_type(c) || c == '\\')
      res += '\\';
    res += c;
  }
  return res;
}
} // namespace

std::string to_string(const Line &line) {
  std::string res;
  for (std::size_t i = 0; i < line.size(); ++i) {
    if (i > 0)
      res += " & ";
    for (std::size_t j = 0; j < line[i].size(); ++j) {
      const Command &cmd = line[i][j];
      if (j > 0)
        res += " | ";
      for (std::size_t k = 0; k < cmd.argv.size(); ++k) {
        if (k > 0)
          res += ' ';
        res += escape(cmd.argv[k]);
      }
      if (cmd.in)
        res += " < " + escape(*cmd.in);
      if (cmd.out)
        res += " > " + escape(*cmd.out);
    }
  }
  return res;
}

std::expected<Line, std::string> parse_args(std::string_view line) {
  auto tokens = tokenizer(line);
  if (!tokens.has_value())
    return std::unexpected(tokens.error());
  return Parser(std::move(*tokens)).line();
}

// int main() {
//   std::string str;
//   std::getline(std::cin, str);
//   tokenizer(str);
//   return 0;
// }
