#include "global.h"
#include "train.h"

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Local
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
static TrainSpeed speed = {{'S','T'}};

void TrainSetSpeed(TrainSpeed val)
{
    speed = val;
    // TODO: IR stuff
}

TrainSpeed TrainGetSpeed(void)
{
    return speed;
}
