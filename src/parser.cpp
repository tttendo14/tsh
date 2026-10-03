#include "parser.hpp"

#include <expected>
#include <iostream>
#include <string>
#include <utility>

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

std::expected<std::vector<Token>, std::string>
tokenizer(std::string_view line) {
  std::vector<Token> args;
  std::size_t pos = 0;
  while (pos < line.size()) {
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
      ++pos;
    }
    if (pos == line.size()) {
      break;
    }

    // static const std::string special_symbols = "&|<>";
    // static const TokenType special_symbols_token[4] = {
    //     TokenType::Lt, TokenType::Gt, TokenType::Pipe, TokenType::Amp};

    if (line[pos] == '&') {
      args.emplace_back(TokenType::Amp, "");
      pos++;
      continue;
    } else if (line[pos] == '|') {
      args.emplace_back(TokenType::Pipe, "");
      pos++;
      continue;
    } else if (line[pos] == '<') {
      args.emplace_back(TokenType::Lt, "");
      pos++;
      continue;
    } else if (line[pos] == '>') {
      args.emplace_back(TokenType::Gt, "");
      pos++;
      continue;
    }
    std::string arg;
    while (pos < line.size() && line[pos] != ' ' && line[pos] != '\t' &&
           line[pos] != '&' && line[pos] != '|' && line[pos] != '<' &&
           line[pos] != '>') {
      if (line[pos] == '\\' && pos + 1 < line.size() &&
          (line[pos + 1] == ' ' || line[pos + 1] == '\t' ||
           line[pos + 1] == '&' || line[pos + 1] == '<' ||
           line[pos + 1] == '>' || line[pos + 1] == '|')) {
        arg += line[pos + 1];
        pos += 2;
      } else if (line[pos] == '\\' && pos + 1 == line.size()) {
        return std::unexpected<std::string>(
            "tokenizer: a valid line cannot end with a '\\'");
      } else {
        arg += line[pos++];
      }
    }
    args.emplace_back(TokenType::Word, std::move(arg));
  }
  return args;
}
} // namespace

// Line parse_args(std::string_view line) {}

// int main() {
//   std::string str;
//   std::getline(std::cin, str);
//   tokenizer(str);
//   return 0;
// }
