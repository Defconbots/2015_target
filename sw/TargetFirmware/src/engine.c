#include "global.h"
#include "engine.h"

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
static EngineSpeed speed = {{'S','T'}};

void EngineSetSpeed(EngineSpeed val)
{
    speed = val;
    // TODO: IR stuff
}

EngineSpeed EngineGetSpeed(void)
{
    return speed;
}
