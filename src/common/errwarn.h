#ifndef JMM_ERRWARN_H
#define JMM_ERRWARN_H

#include <iostream>
#include <cstdlib>

// Change this design later and put into driver.
const int MAX_WARNINGS = 10;
extern int warnings;

// I would like to add a format string here and have fprintf 
// but format string vuln!!
template <typename Arg, typename... Args>
[[noreturn]]
void error(Arg&& arg, Args&&... args)
{
    std::cerr << "Error: " << std::forward<Arg>(arg);
    ((std::cerr << std::forward<Args>(args)), ...);
    std::cerr << '\n';
    exit(1);
}


template <typename Arg, typename... Args>
void warning(Arg&& arg, Args&&... args)
{
    std::cerr << "Warning: " << std::forward<Arg>(arg);
    ((std::cerr << std::forward<Args>(args)), ...);
    std::cerr << '\n';
    warnings++;
    if (warnings > MAX_WARNINGS)
        error("Too many errors to continue.");
}



#endif // !JMM_ERRWARN_H
