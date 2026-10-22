#pragma once

#include "parser.hpp"

enum class CommandExit_t { Exit, Continue };
[[nodiscard]] CommandExit_t run_command(const Command &command);

enum class Builtin_t { NotABuiltIn, Handled, Exit };

Builtin_t run_builtin(const Command &command);
