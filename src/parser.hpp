#pragma once

#include <string>
#include <string_view>
#include <vector>

// An escaped space or tab is part of its argument.
std::vector<std::string> parse_args(std::string_view line);
