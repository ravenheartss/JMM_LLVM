#ifndef JMM_DRIVER_H
#define JMM_DRIVER_H

#include "common/jmm.h"
#include "common/errwarn.h"

#include <memory>
#include <string>

class Driver 
{
    public:
        Driver(std::string file);
        ~Driver();

        bool compile();

    private:
        std::shared_ptr<Lexer> m_lexer;
        std::unique_ptr<Parser> m_parser;
        std::shared_ptr<Logger> m_logger;

        std::string filename;

};
#endif // !JMM_DRIVER_H
