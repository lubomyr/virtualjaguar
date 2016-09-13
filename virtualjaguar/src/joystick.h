//
// Jaguar joystick handler
//

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <stdint.h>

void JoystickInit(void);
void JoystickReset(void);
void JoystickDone(void);
void JoystickWriteByte(uint32_t, uint8_t);
void JoystickWriteWord(uint32_t, uint16_t);
uint8_t JoystickReadByte(uint32_t);
uint16_t JoystickReadWord(uint32_t);
void JoystickExec(void);

#endif
