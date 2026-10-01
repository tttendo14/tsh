#pragma once

#include <string>
#include <vector>

class ShellPath {
public:
  static ShellPath &instance() {
    static ShellPath path{};
    return path;
  }
  void setDirs(std::vector<std::string> dirs_) { dirs = std::move(dirs_); }
  const std::vector<std::string> &getDirs() { return dirs; }

private:
  ShellPath() : dirs({"/bin"}) {}
  // NOTE: Singleton design, delete copy, move & assignment
  ShellPath(const ShellPath &) = delete;            // copy constructor
  ShellPath &operator=(const ShellPath &) = delete; // assignment operator
  ShellPath(ShellPath &&) = delete;                 // move constructor
  ShellPath &operator=(ShellPath &&) = delete; // rvalue assignment operator

  std::vector<std::string> dirs;
};
