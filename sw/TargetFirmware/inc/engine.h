#ifndef ENGINE_H
#define ENGINE_H


typedef struct
{
    uint8_t speed[2];
} EngineSpeed;

void EngineSetSpeed(EngineSpeed val);
EngineSpeed EngineGetSpeed(void);

#endif
