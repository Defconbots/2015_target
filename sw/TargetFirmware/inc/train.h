#ifndef TRAIN_H
#define TRAIN_H


typedef struct
{
    uint8_t speed[2];
} TrainSpeed;

void TrainSetSpeed(TrainSpeed val);
TrainSpeed TrainGetSpeed(void);

#endif
