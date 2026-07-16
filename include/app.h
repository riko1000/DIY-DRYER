#ifndef APP_H
#define APP_H

#include "dryer.h"

class App
{
public:
    void begin();
    void update();

private:
    Dryer dryer;
};

#endif