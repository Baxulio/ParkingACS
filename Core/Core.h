#ifndef CORE_H
#define CORE_H

#include "core_global.h"

#define D0_PIN 29
#define D1_PIN 28
#define BAREER_PIN 6
#define BREAK_PIN 27

#define WIEGANDMAXBITS 35
#define WIEGAND_BIT_INTERVAL_TIMEOUT_USEC 20000

class CORESHARED_EXPORT Core
{

public:
    Core();
};

#endif // CORE_H
