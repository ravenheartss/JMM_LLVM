#include <common/token.h>
#include "driver/driver.h"

#include <memory>

int warnings = 0;
int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [file]\n", argv[0]);
        exit(1);
    }

    std::unique_ptr<Driver> driver = std::make_unique<Driver>(argv[1]);
    driver->compile();
    return 0;
}
