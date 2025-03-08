#include "main.h"

class RaiiPrint {
  public:
    ~RaiiPrint() {
        std::cout << "unwinding works" << std::endl;
    }
};

void initialize() {
    RaiiPrint raii_print;
    // time for the terminal to boot up
    pros::delay(3000);
    // cause a data abort exception.
    // if stack unwinding works, then the RaiiPrint destructor will be called.
    // we'll know it has been called because it will print to the terminal.
    throw "";
}