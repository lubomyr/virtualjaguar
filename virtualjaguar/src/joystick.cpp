//
// Joystick handler
//
// by cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/fixes by James L. Hammons
//

#include "joystick.h"

#include <SDL.h>
#include <time.h>
#include "gpu.h"
#include "gui.h"
#include "jaguar.h"
#include "log.h"
#include "settings.h"
#include "video.h"

#define BUTTON_U		0
#define BUTTON_D		1
#define BUTTON_L		2
#define BUTTON_R		3
#define BUTTON_s		4
#define BUTTON_7		5
#define BUTTON_4		6
#define BUTTON_1		7
#define BUTTON_0		8
#define BUTTON_8		9
#define BUTTON_5		10
#define BUTTON_2		11
#define BUTTON_d		12
#define BUTTON_9		13
#define BUTTON_6		14
#define BUTTON_3		15

#define BUTTON_A		16
#define BUTTON_B		17
#define BUTTON_C		18
#define BUTTON_OPTION	19
#define BUTTON_PAUSE	20

// Global vars

static uint8_t joystick_ram[4];
static uint8_t joypad_0_buttons[21];
static uint8_t joypad_1_buttons[21];
//extern bool finished;
////extern bool showGUI;
bool GUIKeyHeld = false;
extern int start_logging;
int gpu_start_log = 0;
int op_start_log = 0;
int blit_start_log = 0;
int effect_start = 0;
int effect_start2 = 0, effect_start3 = 0, effect_start4 = 0, effect_start5 = 0, effect_start6 = 0;
bool interactiveMode = false;
bool iLeft, iRight, iToggle = false;
bool keyHeld1 = false, keyHeld2 = false, keyHeld3 = false;
int objectPtr = 0;
bool startMemLog = false;
//extern bool doDSPDis, doGPUDis;

bool blitterSingleStep = false;
bool bssGo = false;
bool bssHeld = false;
extern bool running;

void JoystickInit(void)
{
	JoystickReset();
}

void JoystickExec(void)
{
	uint8_t * keystate = SDL_GetKeyState(NULL);

	memset(joypad_0_buttons, 0, 21);
	memset(joypad_1_buttons, 0, 21);
	gpu_start_log = 0;							// Only log while key down!
	effect_start = 0;
	effect_start2 = effect_start3 = effect_start4 = effect_start5 = effect_start6 = 0;
	blit_start_log = 0;
	iLeft = iRight = false;

	if ((keystate[SDLK_LALT] || keystate[SDLK_RALT]) & keystate[SDLK_RETURN])
		ToggleFullscreen();

	// Keybindings in order of U, D, L, R, C, B, A, Op, Pa, 0-9, #, *
//	vjs.p1KeyBindings[0] = sdlemu_getval_int("p1k_up", SDLK_UP);

	if (keystate[vjs.p1KeyBindings[0]])
		joypad_0_buttons[BUTTON_U] = 0x01;
	if (keystate[vjs.p1KeyBindings[1]])
		joypad_0_buttons[BUTTON_D] = 0x01;
	if (keystate[vjs.p1KeyBindings[2]])
		joypad_0_buttons[BUTTON_L] = 0x01;
	if (keystate[vjs.p1KeyBindings[3]])
		joypad_0_buttons[BUTTON_R] = 0x01;
	// The buttons are labelled C,B,A on the controller (going from left to right)
	if (keystate[vjs.p1KeyBindings[4]])
		joypad_0_buttons[BUTTON_C] = 0x01;
	if (keystate[vjs.p1KeyBindings[5]])
		joypad_0_buttons[BUTTON_B] = 0x01;
	if (keystate[vjs.p1KeyBindings[6]])
		joypad_0_buttons[BUTTON_A] = 0x01;
//I may yet move these to O and P...
	if (keystate[vjs.p1KeyBindings[7]])
		joypad_0_buttons[BUTTON_OPTION] = 0x01;
	if (keystate[vjs.p1KeyBindings[8]])
		joypad_0_buttons[BUTTON_PAUSE] = 0x01;

	if (keystate[vjs.p1KeyBindings[9]])
		joypad_0_buttons[BUTTON_0] = 0x01;
	if (keystate[vjs.p1KeyBindings[10]])
		joypad_0_buttons[BUTTON_1] = 0x01;
	if (keystate[vjs.p1KeyBindings[11]])
		joypad_0_buttons[BUTTON_2] = 0x01;
	if (keystate[vjs.p1KeyBindings[12]])
		joypad_0_buttons[BUTTON_3] = 0x01;
	if (keystate[vjs.p1KeyBindings[13]])
		joypad_0_buttons[BUTTON_4] = 0x01;
	if (keystate[vjs.p1KeyBindings[14]])
		joypad_0_buttons[BUTTON_5] = 0x01;
	if (keystate[vjs.p1KeyBindings[15]])
		joypad_0_buttons[BUTTON_6] = 0x01;
	if (keystate[vjs.p1KeyBindings[16]])
		joypad_0_buttons[BUTTON_7] = 0x01;
	if (keystate[vjs.p1KeyBindings[17]])
		joypad_0_buttons[BUTTON_8] = 0x01;
	if (keystate[vjs.p1KeyBindings[18]])
		joypad_0_buttons[BUTTON_9] = 0x01;
	if (keystate[vjs.p1KeyBindings[19]])
		joypad_0_buttons[BUTTON_s] = 0x01;
	if (keystate[vjs.p1KeyBindings[20]])
		joypad_0_buttons[BUTTON_d] = 0x01;

	extern bool debounceRunKey;
	extern void guichan_gui();

    if (keystate[SDLK_ESCAPE])
    {
		//guichan_gui();
		if (!debounceRunKey)
	    	finished = true;
    }
    else
		debounceRunKey = false;

	if (keystate[SDLK_TAB])
	{
		if (!GUIKeyHeld)
			showGUI = !showGUI, GUIKeyHeld = true;
	}
	else
		GUIKeyHeld = false;

	if (keystate[SDLK_q])
		start_logging = 1;
	if (keystate[SDLK_w])
		GPUResetStats();
//	if (keystate[SDLK_u])		jaguar_long_write(0xf1c384,jaguar_long_read(0xf1c384)+1);
	if (keystate[SDLK_d])
		DumpMainMemory();
	if (keystate[SDLK_l])
		gpu_start_log = 1;
	if (keystate[SDLK_o])
		op_start_log = 1;
	if (keystate[SDLK_b])
		blit_start_log = 1;

	if (keystate[SDLK_1])
		effect_start = 1;
	if (keystate[SDLK_2])
		effect_start2 = 1;
	if (keystate[SDLK_3])
		effect_start3 = 1;
	if (keystate[SDLK_4])
		effect_start4 = 1;
	if (keystate[SDLK_5])
		effect_start5 = 1;
	if (keystate[SDLK_6])
		effect_start6 = 1;

	if (keystate[SDLK_i])
		interactiveMode = true;

	if (keystate[SDLK_8] && interactiveMode)
	{
		if (!keyHeld1)
			objectPtr--, keyHeld1 = true;
	}
	else
		keyHeld1 = false;

	if (keystate[SDLK_0] && interactiveMode)
	{
		if (!keyHeld2)
			objectPtr++, keyHeld2 = true;
	}
	else
		keyHeld2 = false;

	if (keystate[SDLK_9] && interactiveMode)
	{
		if (!keyHeld3)
			iToggle = !iToggle, keyHeld3 = true;
	}
	else
		keyHeld3 = false;

	if (keystate[SDLK_e])
		startMemLog = true;
	if (keystate[SDLK_r])
		WriteLog("\n--------> MARK!\n\n");
//	if (keystate[SDLK_t])
//		doDSPDis = true;
//	if (keystate[SDLK_y])
//		doGPUDis = true;

	// BLITTER single step
	if (keystate[SDLK_F5])
		blitterSingleStep = true;

	if (keystate[SDLK_F6])
	{
		if (!bssHeld)
		{
			bssHeld = true;
			bssGo = true;
		}
	}
	else
		bssHeld = false;

	// Joystick support [nwagenaar]

    if (vjs.useJoystick)
    {
		extern SDL_Joystick * joystick1;
		int16_t x = SDL_JoystickGetAxis(joystick1, 0),
			y = SDL_JoystickGetAxis(joystick1, 1);

		if (x > 16384)
			joypad_0_buttons[BUTTON_R] = 0x01;
		if (x < -16384)
			joypad_0_buttons[BUTTON_L] = 0x01;
		if (y > 16384)
			joypad_0_buttons[BUTTON_D] = 0x01;
		if (y < -16384)
			joypad_0_buttons[BUTTON_U] = 0x01;

		if (SDL_JoystickGetButton(joystick1, 0) == SDL_PRESSED)
			joypad_0_buttons[BUTTON_A] = 0x01;
		if (SDL_JoystickGetButton(joystick1, 1) == SDL_PRESSED)
			joypad_0_buttons[BUTTON_B] = 0x01;
		if (SDL_JoystickGetButton(joystick1, 2) == SDL_PRESSED)
			joypad_0_buttons[BUTTON_C] = 0x01;
	}

	// Needed to ensure that the events queue is empty [nwagenaar]
    SDL_PumpEvents();
}

void JoystickReset(void)
{
	memset(joystick_ram, 0x00, 4);
	memset(joypad_0_buttons, 0, 21);
	memset(joypad_1_buttons, 0, 21);
}

void JoystickDone(void)
{
}

uint8_t JoystickReadByte(uint32_t offset)
{
#warning No bounds checking done in JoystickReadByte!
//	extern bool hardwareTypeNTSC;
	offset &= 0x03;

	if (offset == 0)
	{
		uint8_t data = 0x00;
		int pad0Index = joystick_ram[1] & 0x0F;
		int pad1Index = (joystick_ram[1] >> 4) & 0x0F;

// This is bad--we're assuming that a bit is set in the last case. Might not be so!
		if (!(pad0Index & 0x01))
			pad0Index = 0;
		else if (!(pad0Index & 0x02))
			pad0Index = 1;
		else if (!(pad0Index & 0x04))
			pad0Index = 2;
		else
			pad0Index = 3;

		if (!(pad1Index & 0x01))
			pad1Index = 0;
		else if (!(pad1Index & 0x02))
			pad1Index = 1;
		else if (!(pad1Index & 0x04))
			pad1Index = 2;
		else
			pad1Index = 3;

		if (joypad_0_buttons[(pad0Index << 2) + 0])	data |= 0x01;
		if (joypad_0_buttons[(pad0Index << 2) + 1]) data |= 0x02;
		if (joypad_0_buttons[(pad0Index << 2) + 2]) data |= 0x04;
		if (joypad_0_buttons[(pad0Index << 2) + 3]) data |= 0x08;
		if (joypad_1_buttons[(pad1Index << 2) + 0]) data |= 0x10;
		if (joypad_1_buttons[(pad1Index << 2) + 1]) data |= 0x20;
		if (joypad_1_buttons[(pad1Index << 2) + 2]) data |= 0x40;
		if (joypad_1_buttons[(pad1Index << 2) + 3]) data |= 0x80;

		return ~data;
	}
	else if (offset == 3)
	{
		// Hardware ID returns NTSC/PAL identification bit here
		uint8_t data = 0x2F | (vjs.hardwareTypeNTSC ? 0x10 : 0x00);
		int pad0Index = joystick_ram[1] & 0x0F;
//unused		int pad1Index = (joystick_ram[1] >> 4) & 0x0F;

		if (!(pad0Index & 0x01))
		{
			if (joypad_0_buttons[BUTTON_PAUSE])
				data ^= 0x01;
			if (joypad_0_buttons[BUTTON_A])
				data ^= 0x02;
		}
		else if (!(pad0Index & 0x02))
		{
			if (joypad_0_buttons[BUTTON_B])
				data ^= 0x02;
		}
		else if (!(pad0Index & 0x04))
		{
			if (joypad_0_buttons[BUTTON_C])
				data ^= 0x02;
		}
		else
		{
			if (joypad_0_buttons[BUTTON_OPTION])
				data ^= 0x02;
		}
		return data;
	}

	return joystick_ram[offset];
}

uint16_t JoystickReadWord(uint32_t offset)
{
	return ((uint16_t)JoystickReadByte((offset + 0) & 0x03) << 8) | JoystickReadByte((offset + 1) & 0x03);
}

void JoystickWriteByte(uint32_t offset, uint8_t data)
{
	joystick_ram[offset & 0x03] = data;
}

void JoystickWriteWord(uint32_t offset, uint16_t data)
{
#warning No bounds checking done for JoystickWriteWord!
	offset &= 0x03;
	joystick_ram[offset + 0] = (data >> 8) & 0xFF;
	joystick_ram[offset + 1] = data & 0xFF;
}
