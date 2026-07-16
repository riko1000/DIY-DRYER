#include "app.h"

void App::begin()
{
    dryer.begin();

    // Temporary
    dryer.start();
}

void App::update()
{
    dryer.update();
}