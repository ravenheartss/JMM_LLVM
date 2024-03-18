#ifndef JMM_ERRWARN_H
#define JMM_ERRWARN_H

#include <cstdlib>
#include <iostream>

struct Logger {
  // Change this design later and put into driver.
  static constexpr int MAX_WARNINGS = 10;
  int warnings = 0;

  // I would like to add a format string here and have fprintf
  // but format string vuln without any sanitization!
  template <typename Arg, typename... Args>
  [[noreturn]] void error(Arg&& arg, Args&&... args) {
    std::cerr << "Error: " << std::forward<Arg>(arg);
    ((std::cerr << std::forward<Args>(args)), ...);
    std::cerr << '\n';
    exit(1);
  }

  template <typename Arg, typename... Args>
  void warning(Arg&& arg, Args&&... args) {
    std::cerr << "Warning: " << std::forward<Arg>(arg);
    ((std::cerr << std::forward<Args>(args)), ...);
    std::cerr << '\n';
    warnings++;
    if (warnings > MAX_WARNINGS) {
      error("Too many errors to continue.");
    }
  }
};

#endif  // !JMM_ERRWARN_H
