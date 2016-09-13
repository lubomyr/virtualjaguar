//
// GUI.CPP
//
// Graphical User Interface support
// by James L. Hammons
//

#include <stdarg.h>
#include <sys/types.h>								// For MacOS <dirent.h> dependency
#include <dirent.h>
#include <SDL.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ctype.h>									// For toupper()
#include "jaguar.h"
#include "eeprom.h"
#include "log.h"
#include "joystick.h"
#include "settings.h"
#include "tom.h"
#include "video.h"
#include "font1.h"
#include "crc32.h"
#include "zlib.h"
#include "unzip.h"
#include "gui.h"
#include "file.h"

#include "jagbios.h"
#include "jagcdbios.h"
#include "jagstub2bios.h"

using namespace std;								// For STL stuff

// Private function prototypes

class Window;										// Forward declaration...

//void DrawTransparentBitmap(int16_t * screen, uint32_t x, uint32_t y, uint16_t * bitmap, uint8_t * alpha = NULL);
//void DrawStringTrans(int16_t * screen, uint32_t x, uint32_t y, uint16_t color, uint8_t opacity, const char * text, ...);
//void DrawStringOpaque(int16_t * screen, uint32_t x, uint32_t y, uint16_t color1, uint16_t color2, const char * text, ...);
//void DrawString(int16_t * screen, uint32_t x, uint32_t y, bool invert, const char * text, ...);
void DrawTransparentBitmap(uint32_t * screen, uint32_t x, uint32_t y, uint32_t * bitmap, uint8_t * alpha = NULL);
void DrawStringTrans(uint32_t * screen, uint32_t x, uint32_t y, uint32_t color, uint8_t opacity, const char * text, ...);
void DrawStringOpaque(uint32_t * screen, uint32_t x, uint32_t y, uint32_t color1, uint32_t color2, const char * text, ...);
void DrawString(uint32_t * screen, uint32_t x, uint32_t y, bool invert, const char * text, ...);
Window * LoadROM(void);
Window * ResetJaguar(void);
Window * ResetJaguarCD(void);
Window * RunEmu(void);
Window * Quit(void);
Window * About(void);
Window * MiscOptions(void);


// Local global variables

bool showGUI = false;
bool exitGUI = false;								// GUI (emulator) done variable
bool finished = false;
extern bool jaguarCartInserted;
int mouseX, mouseY;
//uint16_t background[1280 * 256];						// GUI background buffer
uint32_t background[1280 * 256];						// GUI background buffer

//NOTE: 32-bit pixels are in the format of ABGR...
uint32_t mousePic[] = {
	6, 8,

	0xFF00FF00,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,		// +
	0xFF00C600,0xFF00FF00,0x00000000,0x00000000,0x00000000,0x00000000,		// @+
	0xFF00C600,0xFF00FF00,0xFF00FF00,0x00000000,0x00000000,0x00000000,		// @++
	0xFF00C600,0xFF00C600,0xFF00FF00,0xFF00FF00,0x00000000,0x00000000,		// @@++
	0xFF00C600,0xFF00C600,0xFF00FF00,0xFF00FF00,0xFF00FF00,0x00000000,		// @@+++
	0xFF00C600,0xFF00C600,0xFF00C600,0xFF00FF00,0xFF00FF00,0xFF00FF00,		// @@@+++
	0xFF00C600,0xFF00C600,0xFF00C600,0x00000000,0x00000000,0x00000000,		// @@@
	0xFF00C600,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000		// @
/*
	0x03E0,0x0000,0x0000,0x0000,0x0000,0x0000,		// +
	0x0300,0x03E0,0x0000,0x0000,0x0000,0x0000,		// @+
	0x0300,0x03E0,0x03E0,0x0000,0x0000,0x0000,		// @++
	0x0300,0x0300,0x03E0,0x03E0,0x0000,0x0000,		// @@++
	0x0300,0x0300,0x03E0,0x03E0,0x03E0,0x0000,		// @@+++
	0x0300,0x0300,0x0300,0x03E0,0x03E0,0x03E0,		// @@@+++
	0x0300,0x0300,0x0300,0x0000,0x0000,0x0000,		// @@@
	0x0300,0x0000,0x0000,0x0000,0x0000,0x0000		// @
*/
/*
	0xFFFF,0x0000,0x0000,0x0000,0x0000,0x0000,		// +
	0xE318,0xFFFF,0x0000,0x0000,0x0000,0x0000,		// @+
	0xE318,0xFFFF,0xFFFF,0x0000,0x0000,0x0000,		// @++
	0xE318,0xE318,0xFFFF,0xFFFF,0x0000,0x0000,		// @@++
	0xE318,0xE318,0xFFFF,0xFFFF,0xFFFF,0x0000,		// @@+++
	0xE318,0xE318,0xE318,0xFFFF,0xFFFF,0xFFFF,		// @@@+++
	0xE318,0xE318,0xE318,0x0000,0x0000,0x0000,		// @@@
	0xE318,0x0000,0x0000,0x0000,0x0000,0x0000		// @
*/
};
// 1 111 00 11 100 1 1100 -> F39C
// 1 100 00 10 000 1 0000 -> C210
// 1 110 00 11 000 1 1000 -> E318
// 0 000 00 11 111 0 0000 -> 03E0
// 0 000 00 11 000 0 0000 -> 0300 -> FF00C600

uint32_t closeBox[] = {
	7, 7,

//4B5E -> 010010 11010 11110 -> 0100 1001 1101 0110 1111 0111 -> 49 D6 F7
//0217 -> 000000 10000 10111 -> 0000 0000 1000 0100 1011 1101 -> 00 84 BD
	0x00000000,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0x00000000,		//  +++++
	0xFFF7D649,0xFFFFFFFF,0x00000000,0x00000000,0x00000000,0xFFFFFFFF,0xFFBD8400,		// +@   @.
	0xFFF7D649,0x00000000,0xFFFFFFFF,0x00000000,0xFFFFFFFF,0x00000000,0xFFBD8400,		// + @ @ .
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @  .
	0xFFF7D649,0x00000000,0xFFFFFFFF,0x00000000,0xFFFFFFFF,0x00000000,0xFFBD8400,		// + @ @ .
	0xFFF7D649,0xFFFFFFFF,0x00000000,0x00000000,0x00000000,0xFFFFFFFF,0xFFBD8400,		// +@   @.
	0x00000000,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0x00000000		//  .....
/*
	0x0000,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x4B5E,0x0000,		//  +++++
	0x4B5E,0xFFFF,0x0000,0x0000,0x0000,0xFFFF,0x0217,		// +@   @.
	0x4B5E,0x0000,0xFFFF,0x0000,0xFFFF,0x0000,0x0217,		// + @ @ .
	0x4B5E,0x0000,0x0000,0xFFFF,0x0000,0x0000,0x0217,		// +  @  .
	0x4B5E,0x0000,0xFFFF,0x0000,0xFFFF,0x0000,0x0217,		// + @ @ .
	0x4B5E,0xFFFF,0x0000,0x0000,0x0000,0xFFFF,0x0217,		// +@   @.
	0x0000,0x0217,0x0217,0x0217,0x0217,0x0217,0x0000		//  .....
*/
};

uint32_t upArrowBox[] = {
	8, 8,

	0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,		// ++++++++
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFF7D649,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0xFFBD8400,		// + @@@@ .
	0xFFF7D649,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFBD8400,		// +@@@@@@.
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400			// ........
};

uint32_t downArrowBox[] = {
	8, 8,

	0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,		// ++++++++
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFF7D649,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFBD8400,		// +@@@@@@.
	0xFFF7D649,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0xFFBD8400,		// + @@@@ .
	0xFFF7D649,0x00000000,0x00000000,0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x00000000,0xFFBD8400,		// +  @@  .
	0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400			// ........
};

uint32_t pushButtonUp[] = {
	8, 8,

	0x00000000, 0xFF1B1B1B, 0xFF545477, 0xFF525292, 0xFF474787, 0xFF363659, 0xFF0F0F0F, 0x00000000,
	0xFF1B1B1C, 0xFF6666A7, 0xFF393995, 0xFF343492, 0xFF2F2F90, 0xFF2C2C90, 0xFF3B3B7E, 0xFF0F0F0F,
	0xFF555578, 0xFF3A3A95, 0xFF353594, 0xFF303091, 0xFF2D2D8F, 0xFF2B2B90, 0xFF2A2A92, 0xFF333358,
	0xFF545493, 0xFF343492, 0xFF303092, 0xFF2D2D8B, 0xFF2B2B8A, 0xFF2A2A8D, 0xFF292994, 0xFF3D3D83,
	0xFF484889, 0xFF2F2F90, 0xFF2D2D8F, 0xFF2B2B8A, 0xFF2A2A89, 0xFF29298E, 0xFF292998, 0xFF3D3D84,
	0xFF37375A, 0xFF2C2C90, 0xFF2B2B90, 0xFF2A2A8D, 0xFF29298E, 0xFF292995, 0xFF29299D, 0xFF34345B,
	0xFF0E0E0E, 0xFF3E3E7F, 0xFF2A2A92, 0xFF292994, 0xFF292998, 0xFF29299D, 0xFF3C3C88, 0xFF0E0E0E,
	0x00000000, 0xFF0D0D0D, 0xFF343456, 0xFF3D3D80, 0xFF3D3D82, 0xFF333358, 0xFF0D0D0D, 0x00000000
};

uint8_t pbuAlpha[] = {
	0xFF, 0xE4, 0xA0, 0x99, 0xA4, 0xBE, 0xF0, 0xFF, 
	0xE3, 0x85, 0x00, 0x00, 0x00, 0x00, 0xAF, 0xF0, 
	0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
	0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAD, 
	0xA3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAC, 
	0xBD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBF, 
	0xF1, 0xAD, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xF1, 
	0xFF, 0xF2, 0xC0, 0xAD, 0xAD, 0xC0, 0xF2, 0xFF
};

uint32_t pushButtonDown[] = {
	8, 8,

	0x00000000, 0xFF1B1B1B, 0xFF8B8B90, 0xFF8C8CAF, 0xFF767699, 0xFF56565B, 0xFF0F0F0F, 0x00000000,
	0xFF1B1B1B, 0xFFB8B8D6, 0xFF5555E4, 0xFF4444F2, 0xFF4040F1, 0xFF4141D5, 0xFF626282, 0xFF0F0F0F,
	0xFF8C8C91, 0xFF5555E4, 0xFF4444EF, 0xFF3E3EDC, 0xFF3B3BDB, 0xFF3D3DEC, 0xFF3E3ED4, 0xFF4B4B51,
	0xFF8D8DB1, 0xFF4444F2, 0xFF3E3EDC, 0xFF3E3EDD, 0xFF3C3CDC, 0xFF3939D9, 0xFF3C3CF3, 0xFF59597E,
	0xFF77779B, 0xFF4141F1, 0xFF3B3BDB, 0xFF3C3CDC, 0xFF3B3BDC, 0xFF3838D9, 0xFF3C3CF5, 0xFF595980,
	0xFF57575D, 0xFF4242D8, 0xFF3D3DEC, 0xFF3939D9, 0xFF3838D9, 0xFF3C3CEF, 0xFF3D3DDC, 0xFF4C4C52,
	0xFF101010, 0xFF636385, 0xFF3E3ED8, 0xFF3D3DF4, 0xFF3D3DF6, 0xFF3D3DDD, 0xFF5D5D83, 0xFF101010,
	0x00000000, 0xFF101010, 0xFF4E4E55, 0xFF5B5B83, 0xFF5B5B84, 0xFF4D4D54, 0xFF101010, 0x00000000
};

uint8_t pbdAlpha[] = {
	0xFF, 0xE4, 0x72, 0x68, 0x7E, 0xA7, 0xF0, 0xFF, 
	0xE4, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x93, 0xF0, 
	0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB2, 
	0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 
	0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 
	0xA6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB1, 
	0xEF, 0x91, 0x00, 0x00, 0x00, 0x00, 0x96, 0xEF, 
	0xFF, 0xEF, 0xAE, 0x98, 0x97, 0xAF, 0xEF, 0xFF
};

uint32_t slideSwitchUp[] = {
	8, 16,

//0C7F -> 000011 00011 11111 -> 0000 1100 0001 1000 1111 1111 -> 0C 18 FF
	0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,		// ++++++++
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400			// ........
};

uint32_t slideSwitchDown[] = {
	8, 16,

	0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,0xFFF7D649,		// ++++++++
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0xFFBD8400,		// +.......
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFF7D649,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFFF180C,0xFFBD8400,		// +      .
	0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400,0xFFBD8400			// ........
};

/*uint32_t [] = {
	8, 8,

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,		// ........
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000			// ........
};*/

char separator[] = "--------------------------------------------------------";

//
// Case insensitive string compare function
// Taken straight out of Thinking In C++ by Bruce Eckel. Thanks Bruce!
//

int stringCmpi(const string &s1, const string &s2)
{
	// Select the first element of each string:
	string::const_iterator p1 = s1.begin(), p2 = s2.begin();

	while (p1 != s1.end() && p2 != s2.end())		// Don’t run past the end
	{
		if (toupper(*p1) != toupper(*p2))			// Compare upper-cased chars
			return (toupper(*p1) < toupper(*p2) ? -1 : 1);// Report which was lexically greater

		p1++;
		p2++;
	}

	// If they match up to the detected eos, say which was longer. Return 0 if the same.
	return s2.size() - s1.size();
}

//
// Local GUI classes
//

enum { WINDOW_CLOSE, MENU_ITEM_CHOSEN };

class Element
{
	public:
		Element(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0)
			{ extents.x = x, extents.y = y, extents.w = w, extents.h = h; }
		virtual void HandleKey(SDLKey key) = 0;		// These are "pure" virtual functions...
		virtual void HandleMouseMove(uint32_t x, uint32_t y) = 0;
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown) = 0;
		virtual void Draw(uint32_t, uint32_t) = 0;
		virtual void Notify(Element *) = 0;
//Needed?		virtual ~Element() = 0;
//We're not allocating anything in the base class, so the answer would be NO.
		bool Inside(uint32_t x, uint32_t y);
		// Class method
//		static void SetScreenAndPitch(int16_t * s, uint32_t p) { screenBuffer = s, pitch = p; }
		static void SetScreenAndPitch(uint32_t * s, uint32_t p) { screenBuffer = s, pitch = p; }

	protected:
		SDL_Rect extents;
		uint32_t state;
		// Class variables...
//		static int16_t * screenBuffer;
		static uint32_t * screenBuffer;
		static uint32_t pitch;
};

// Initialize class variables (Element)
//int16_t * Element::screenBuffer = NULL;
uint32_t * Element::screenBuffer = NULL;
uint32_t Element::pitch = 0;

bool Element::Inside(uint32_t x, uint32_t y)
{
	return (x >= (uint32_t)extents.x && x < (uint32_t)(extents.x + extents.w)
		&& y >= (uint32_t)extents.y && y < (uint32_t)(extents.y + extents.h) ? true : false);
}


class Button: public Element
{
	public:
		Button(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFFFFFF),
			bgColor(0xFF00FF00), pic(NULL), elementToTell(NULL) {}
		Button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t * p): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFFFFFF),
			bgColor(0xFF00FF00), pic(p), elementToTell(NULL) {}
		Button(uint32_t x, uint32_t y, uint32_t * p): Element(x, y, 0, 0),
			activated(false), clicked(false), inside(false), fgColor(0xFFFFFFFF),
			bgColor(0xFF00FF00), pic(p), elementToTell(NULL)
			{ if (pic) extents.w = pic[0], extents.h = pic[1]; }
		Button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string s): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFFFFFF),
			bgColor(0xFF00FF00), pic(NULL), text(s), elementToTell(NULL) {}
		Button(uint32_t x, uint32_t y, string s): Element(x, y, 0, 8),
			activated(false), clicked(false), inside(false), fgColor(0xFFFFFFFF),
			bgColor(0xFF00FF00), pic(NULL), text(s), elementToTell(NULL)
			{ extents.w = s.length() * 8; }
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32_t x, uint32_t y);
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown);
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element *) {}
		bool ButtonClicked(void) { return activated; }
		void SetNotificationElement(Element * e) { elementToTell = e; }

	protected:
		bool activated, clicked, inside;
		uint32_t fgColor, bgColor;
		uint32_t * pic;
		string text;
		Element * elementToTell;
};

void Button::HandleMouseMove(uint32_t x, uint32_t y)
{
	inside = Inside(x, y);
}

void Button::HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown)
{
	if (inside)
	{
		if (mouseDown)
			clicked = true;

		if (clicked && !mouseDown)
		{
			clicked = false, activated = true;

			// Send a message that we're activated (if there's someone to tell, that is)
			if (elementToTell)
				elementToTell->Notify(this);
		}
	}
	else
		clicked = activated = false;
}

void Button::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
	uint32_t addr = (extents.x + offsetX) + ((extents.y + offsetY) * pitch);

	for(uint32_t y=0; y<extents.h; y++)
	{
		for(uint32_t x=0; x<extents.w; x++)
		{
			// Doesn't clip in y axis! !!! FIX !!!
			if (extents.x + x < pitch)
				screenBuffer[addr + x + (y * pitch)] 
//					= (clicked && inside ? fgColor : (inside ? 0x43F0 : bgColor));
//43F0 -> 010000 11111 10000 -> 0100 0001 1111 1111 1000 0100 -> 41 FF 84
					= (clicked && inside ? fgColor : (inside ? 0xFF84FF41 : bgColor));
		}
	}

	if (pic != NULL)
		DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, pic);

	if (text.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
}


class PushButton: public Element
{
// How to handle?
// Save state externally?
//We pass in a state variable if we want to track it externally, otherwise we use our own
//internal state var. Still need to do some kind of callback for pushbuttons that do things
//like change from fullscreen to windowed... !!! FIX !!!

	public:
//		PushButton(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0): Element(x, y, w, h),
//			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
//			bgColor(0x03E0), pic(NULL), elementToTell(NULL) {}
		PushButton(uint32_t x, uint32_t y, bool * st, string s): Element(x, y, 8, 8), state(st),
			inside(false), text(s) { if (st == NULL) state = &internalState; }
/*		Button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t * p): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(p), elementToTell(NULL) {}
		Button(uint32_t x, uint32_t y, uint32_t * p): Element(x, y, 0, 0),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(p), elementToTell(NULL)
			{ if (pic) extents.w = pic[0], extents.h = pic[1]; }
		Button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, string s): Element(x, y, w, h),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(NULL), text(s), elementToTell(NULL) {}
		PushButton(uint32_t x, uint32_t y, string s): Element(x, y, 0, 8),
			activated(false), clicked(false), inside(false), fgColor(0xFFFF),
			bgColor(0x03E0), pic(NULL), text(s), elementToTell(NULL)
			{ extents.w = s.length() * 8; }*/
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32_t x, uint32_t y);
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown);
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element *) {}
//		bool ButtonClicked(void) { return activated; }
//		void SetNotificationElement(Element * e) { elementToTell = e; }

	protected:
		bool * state;
		bool inside;
//		bool activated, clicked, inside;
//		uint16_t fgColor, bgColor;
//		uint32_t * pic;
		string text;
//		Element * elementToTell;
		bool internalState;
};

void PushButton::HandleMouseMove(uint32_t x, uint32_t y)
{
	inside = Inside(x, y);
}

void PushButton::HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown)
{
	if (inside && mouseDown)
	{
/*		if (mouseDown)
			clicked = true;

		if (clicked && !mouseDown)
		{
			clicked = false, activated = true;

			// Send a message that we're activated (if there's someone to tell, that is)
			if (elementToTell)
				elementToTell->Notify(this);
		}*/
		*state = !(*state);
	}
//	else
//		clicked = activated = false;
}

void PushButton::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
/*	uint32_t addr = (extents.x + offsetX) + ((extents.y + offsetY) * pitch);

	for(uint32_t y=0; y<extents.h; y++)
	{
		for(uint32_t x=0; x<extents.w; x++)
		{
			// Doesn't clip in y axis! !!! FIX !!!
			if (extents.x + x < pitch)
				screenBuffer[addr + x + (y * pitch)] 
					= (clicked && inside ? fgColor : (inside ? 0x43F0 : bgColor));
		}
	}*/

//	DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, (*state ? pushButtonDown : pushButtonUp));
	DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, (*state ? pushButtonDown : pushButtonUp), (*state ? pbdAlpha : pbuAlpha));
	if (text.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX + 12, extents.y + offsetY, false, "%s", text.c_str());
}


class SlideSwitch: public Element
{
// How to handle?
// Save state externally?
//Seems to be handled the same as PushButton, but without sanity checks. !!! FIX !!!

	public:
		SlideSwitch(uint32_t x, uint32_t y, bool * st, string s1, string s2): Element(x, y, 8, 16), state(st),
			inside(false), text1(s1), text2(s2) {}
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32_t x, uint32_t y);
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown);
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element *) {}
//		bool ButtonClicked(void) { return activated; }
//		void SetNotificationElement(Element * e) { elementToTell = e; }

	protected:
		bool * state;
		bool inside;
//		bool activated, clicked, inside;
//		uint16_t fgColor, bgColor;
//		uint32_t * pic;
		string text1, text2;
//		Element * elementToTell;
};

void SlideSwitch::HandleMouseMove(uint32_t x, uint32_t y)
{
	inside = Inside(x, y);
}

void SlideSwitch::HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown)
{
	if (inside && mouseDown)
	{
/*		if (mouseDown)
			clicked = true;

		if (clicked && !mouseDown)
		{
			clicked = false, activated = true;

			// Send a message that we're activated (if there's someone to tell, that is)
			if (elementToTell)
				elementToTell->Notify(this);
		}*/
		*state = !(*state);
	}
//	else
//		clicked = activated = false;
}

void SlideSwitch::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
	DrawTransparentBitmap(screenBuffer, extents.x + offsetX, extents.y + offsetY, (*state ? slideSwitchDown : slideSwitchUp));
	if (text1.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX + 12, extents.y + offsetY, false, "%s", text1.c_str());
	if (text2.length() > 0)
		DrawString(screenBuffer, extents.x + offsetX + 12, extents.y + offsetY + 8, false, "%s", text2.c_str());
}


class Window: public Element
{
	public:
/*		Window(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0):	Element(x, y, w, h),
			fgColor(0x4FF0), bgColor(0xFE10)
			{ close = new Button(w - 8, 1, closeBox); list.push_back(close); }*/
		Window(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0,
			void (* f)(Element *) = NULL): Element(x, y, w, h),
//			/*clicked(false), inside(false),*/ fgColor(0x4FF0), bgColor(0x1E10),
//4FF0 -> 010011 11111 10000 -> 0100 1101 1111 1111 1000 0100 -> 4D FF 84
//1E10 -> 000111 10000 10000 -> 0001 1111 1000 0100 1000 0100 -> 1F 84 84
			/*clicked(false), inside(false),*/ fgColor(0xFF84FF4D), bgColor(0xFF84841F),
			handler(f)
			{ close = new Button(w - 8, 1, closeBox); list.push_back(close);
			  close->SetNotificationElement(this); }
		virtual ~Window();
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32_t x, uint32_t y);
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown);
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element * e);
		void AddElement(Element * e);
//		bool WindowActive(void) { return true; }//return !close->ButtonClicked(); }

	protected:
//		bool clicked, inside;
		uint32_t fgColor, bgColor;
		void (* handler)(Element *);
		Button * close;
//We have to use a list of Element *pointers* because we can't make a list that will hold
//all the different object types in the same list...
		vector<Element *> list;
};

Window::~Window()
{
	for(uint32_t i=0; i<list.size(); i++)
		if (list[i])
			delete list[i];
}

void Window::HandleKey(SDLKey key)
{
	if (key == SDLK_ESCAPE)
	{
		SDL_Event event;
		event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
		SDL_PushEvent(&event);
	}

	// Handle the items this window contains...
	for(uint32_t i=0; i<list.size(); i++)
		// Make coords relative to upper right corner of this window...
		list[i]->HandleKey(key);
}

void Window::HandleMouseMove(uint32_t x, uint32_t y)
{
	// Handle the items this window contains...
	for(uint32_t i=0; i<list.size(); i++)
		// Make coords relative to upper right corner of this window...
		list[i]->HandleMouseMove(x - extents.x, y - extents.y);
}

void Window::HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown)
{
	// Handle the items this window contains...
	for(uint32_t i=0; i<list.size(); i++)
		// Make coords relative to upper right corner of this window...
		list[i]->HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
}

void Window::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
	uint32_t addr = (extents.x + offsetX) + ((extents.y + offsetY) * pitch);

	for(uint32_t y=0; y<extents.h; y++)
	{
		for(uint32_t x=0; x<extents.w; x++)
		{
			// Doesn't clip in y axis! !!! FIX !!!
			if (extents.x + x < pitch)
				screenBuffer[addr + x + (y * pitch)] = bgColor;
		}
	}

	// Handle the items this window contains...
	for(uint32_t i=0; i<list.size(); i++)
		list[i]->Draw(extents.x, extents.y);
}

void Window::AddElement(Element * e)
{
	list.push_back(e);
}

void Window::Notify(Element * e)
{
	if (e == close)
	{
		SDL_Event event;
		event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
		SDL_PushEvent(&event);
	}
}


class Text: public Element
{
	public:
//		Text(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0): Element(x, y, w, h),
//			fgColor(0x4FF0), bgColor(0xFE10) {}
//		Text(uint32_t x, uint32_t y, string s, uint16_t fg = 0x4FF0, uint16_t bg = 0xFE10): Element(x, y, 0, 0),
//			fgColor(fg), bgColor(bg), text(s) {}
//4FF0 -> 010011 11111 10000 -> 0100 1101 1111 1111 1000 0100 -> 4D FF 84
//FE10 -> 111111 10000 10000 -> 1111 1111 1000 0100 1000 0100 -> FF 84 84
		Text(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0): Element(x, y, w, h),
			fgColor(0xFF84FF4D), bgColor(0xFF8484FF) {}
		Text(uint32_t x, uint32_t y, string s, uint32_t fg = 0xFF84FF4D, uint32_t bg = 0xFF8484FF): Element(x, y, 0, 0),
			fgColor(fg), bgColor(bg), text(s) {}
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32_t x, uint32_t y) {}
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown) {}
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element *) {}

	protected:
		uint32_t fgColor, bgColor;
		string text;
};

void Text::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
	if (text.length() > 0)
//		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY, false, "%s", text.c_str());
		DrawStringOpaque(screenBuffer, extents.x + offsetX, extents.y + offsetY, fgColor, bgColor, "%s", text.c_str());
}


class ListBox: public Element
//class ListBox: public Window
{
	public:
//		ListBox(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0): Element(x, y, w, h),
		ListBox(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0);//: Window(x, y, w, h),
//		windowPtr(0), cursor(0), limit(0), charWidth((w / 8) - 1), charHeight(h / 8),
//		elementToTell(NULL), upArrow(w - 8, 0, upArrowBox),
//		downArrow(w - 8, h - 8, downArrowBox), upArrow2(w - 8, h - 16, upArrowBox) {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32_t x, uint32_t y);
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown);
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element * e);
		void SetNotificationElement(Element * e) { elementToTell = e; }
		void AddItem(string s);
		string GetSelectedItem(void);

	protected:
		bool thumbClicked;
		uint32_t windowPtr, cursor, limit;
		uint32_t charWidth, charHeight;				// Box width/height in characters
		Element * elementToTell;
		Button upArrow, downArrow, upArrow2;
		vector<string> item;

	private:
		uint32_t yRelativePoint;
};

ListBox::ListBox(uint32_t x, uint32_t y, uint32_t w, uint32_t h): Element(x, y, w, h),
	thumbClicked(false), windowPtr(0), cursor(0), limit(0), charWidth((w / 8) - 1),
	charHeight(h / 8), elementToTell(NULL), upArrow(w - 8, 0, upArrowBox),
	downArrow(w - 8, h - 8, downArrowBox), upArrow2(w - 8, h - 16, upArrowBox)
{
	upArrow.SetNotificationElement(this);
	downArrow.SetNotificationElement(this);
	upArrow2.SetNotificationElement(this);
	extents.w -= 8;									// Make room for scrollbar...
}

void ListBox::HandleKey(SDLKey key)
{
	if (key == SDLK_DOWN)
	{
		if (cursor != limit - 1)	// Cursor is within its window
			cursor++;
		else						// Otherwise, scroll the window...
		{
			if (cursor + windowPtr != item.size() - 1)
				windowPtr++;
		}
	}
	else if (key == SDLK_UP)
	{
		if (cursor != 0)
			cursor--;
		else
		{
			if (windowPtr != 0)
				windowPtr--;
		}
	}
	else if (key == SDLK_PAGEDOWN)
	{
		if (cursor != limit - 1)
			cursor = limit - 1;
		else
		{
			windowPtr += limit;
			if (windowPtr > item.size() - limit)
				windowPtr = item.size() - limit;
		}
	}
	else if (key == SDLK_PAGEUP)
	{
		if (cursor != 0)
			cursor = 0;
		else
		{
			if (windowPtr < limit)
				windowPtr = 0;
			else
				windowPtr -= limit;
		}
	}
	else if (key >= SDLK_a && key <= SDLK_z)
	{
		// Advance cursor to filename with first letter pressed...
		uint8_t which = (key - SDLK_a) + 65;	// Convert key to A-Z char

		for(uint32_t i=0; i<item.size(); i++)
		{
			if ((item[i][0] & 0xDF) == which)
			{
				cursor = i - windowPtr;
				if (i > windowPtr + limit - 1)
					windowPtr = i - limit + 1, cursor = limit - 1;
				if (i < windowPtr)
					windowPtr = i, cursor = 0;
				break;
			}
		}
	}
}

void ListBox::HandleMouseMove(uint32_t x, uint32_t y)
{
	upArrow.HandleMouseMove(x - extents.x, y - extents.y);
	downArrow.HandleMouseMove(x - extents.x, y - extents.y);
	upArrow2.HandleMouseMove(x - extents.x, y - extents.y);

	if (thumbClicked)
	{
		uint32_t sbHeight = extents.h - 24,
			thumb = (uint32_t)(((float)limit / (float)item.size()) * (float)sbHeight);

//yRelativePoint is the spot on the thumb where we clicked...
		int32_t newThumbStart = y - yRelativePoint;

		if (newThumbStart < 0)
			newThumbStart = 0;

		if ((uint32_t)newThumbStart > sbHeight - thumb)
			newThumbStart = sbHeight - thumb;

		windowPtr = (uint32_t)(((float)newThumbStart / (float)sbHeight) * (float)item.size());
//Check for cursor bounds as well... Or do we need to???
//Actually, we don't...!
	}
}

void ListBox::HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown)
{
	if (Inside(x, y) && mouseDown)
	{
		// Why do we have to do this??? (- extents.y?)
		// I guess it's because only the Window class has offsetting implemented... !!! FIX !!!
		cursor = (y - extents.y) / 8;
	}

	// Check for a hit on the scrollbar...
	if (x > (uint32_t)(extents.x + extents.w) && x <= (uint32_t)(extents.x + extents.w + 8)
		&& y > (uint32_t)(extents.y + 8) && y <= (uint32_t)(extents.y + extents.h - 16))
	{
		if (mouseDown)
		{
// This shiaut should be calculated in AddItem(), not here... (or in Draw() for that matter)
			uint32_t sbHeight = extents.h - 24,
				thumb = (uint32_t)(((float)limit / (float)item.size()) * (float)sbHeight),
				thumbStart = (uint32_t)(((float)windowPtr / (float)item.size()) * (float)sbHeight);

			// Did we hit the thumb?
			if (y >= (extents.y + 8 + thumbStart) && y < (extents.y + 8 + thumbStart + thumb))
				thumbClicked = true, yRelativePoint = y - thumbStart;
		}
//Seems that this is useless--never reached except in rare cases and that the code outside is
//more effective...
//		else
//			thumbClicked = false;
	}

	if (!mouseDown)
		thumbClicked = false;

	upArrow.HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
	downArrow.HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
	upArrow2.HandleMouseButton(x - extents.x, y - extents.y, mouseDown);
}

void ListBox::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
	for(uint32_t i=0; i<limit; i++)
	{
		// Strip off the extension
		// (extension stripping should be an option, not default!)
		string s(item[windowPtr + i], 0, item[windowPtr + i].length() - 4);
		DrawString(screenBuffer, extents.x + offsetX, extents.y + offsetY + i*8,
			(cursor == i ? true : false), "%-*.*s", charWidth, charWidth, s.c_str());
	}

	upArrow.Draw(extents.x + offsetX, extents.y + offsetY);
	downArrow.Draw(extents.x + offsetX, extents.y + offsetY);
	upArrow2.Draw(extents.x + offsetX, extents.y + offsetY);

	uint32_t sbHeight = extents.h - 24,
		thumb = (uint32_t)(((float)limit / (float)item.size()) * (float)sbHeight),
		thumbStart = (uint32_t)(((float)windowPtr / (float)item.size()) * (float)sbHeight);

	for(uint32_t y=extents.y+offsetY+8; y<extents.y+offsetY+extents.h-16; y++)
	{
//		for(uint32_t x=extents.x+offsetX+extents.w-8; x<extents.x+offsetX+extents.w; x++)
		for(uint32_t x=extents.x+offsetX+extents.w; x<extents.x+offsetX+extents.w+8; x++)
		{
			if (y >= thumbStart + (extents.y+offsetY+8) && y < thumbStart + thumb + (extents.y+offsetY+8))
//				screenBuffer[x + (y * pitch)] = (thumbClicked ? 0x458E : 0xFFFF);
//458E -> 01 0001  0 1100  0 1110 -> 0100 0101  0110 0011  0111 0011 -> 45 63 73
				screenBuffer[x + (y * pitch)] = (thumbClicked ? 0xFF736345 : 0xFFFFFFFF);
			else
//				screenBuffer[x + (y * pitch)] = 0x0200;
//0200 -> 000000 10000 00000 -> 00 1000 0100 00
				screenBuffer[x + (y * pitch)] = 0xFF008400;
		}
	}
}

void ListBox::Notify(Element * e)
{
	if (e == &upArrow || e == &upArrow2)
	{
		if (windowPtr != 0)
		{
			windowPtr--;

			if (cursor < limit - 1)
				cursor++;
		}
	}
	else if (e == &downArrow)
	{
		if (windowPtr < item.size() - limit)
		{
			windowPtr++;

			if (cursor != 0)
				cursor--;
		}
	}
}

void ListBox::AddItem(string s)
{
	// Do a simple insertion sort
	bool inserted = false;

	for(vector<string>::iterator i=item.begin(); i<item.end(); i++)
	{
		if (stringCmpi(s, *i) == -1)
		{
			item.insert(i, s);
			inserted = true;
			break;
		}
	}

	if (!inserted)
		item.push_back(s);

	limit = (item.size() > charHeight ? charHeight : item.size());
}

string ListBox::GetSelectedItem(void)
{
	return item[windowPtr + cursor];
}


class FileList: public Window
{
	public:
		FileList(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0);
		virtual ~FileList() {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32_t x, uint32_t y) { Window::HandleMouseMove(x, y); }
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown) { Window::HandleMouseButton(x, y, mouseDown); }
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0) { Window::Draw(offsetX, offsetY); }
		virtual void Notify(Element * e);

	protected:
		ListBox * files;
		Button * load;
};

//Need 4 buttons, one scrollbar...
FileList::FileList(uint32_t x, uint32_t y, uint32_t w, uint32_t h): Window(x, y, w, h)
{
	files = new ListBox(8, 8, w - 16, h - 32);
	AddElement(files);
	load = new Button(8, h - 16, " Load ");
	AddElement(load);
	load->SetNotificationElement(this);

//!!! FIX !!! Directory might not exist--this shouldn't cause VJ to crash!
	DIR * dp = opendir(vjs.ROMPath);
	dirent * de;

	while ((de = readdir(dp)) != NULL)
	{
		char * ext = strrchr(de->d_name, '.');

		if (ext != NULL)
			if (strcasecmp(ext, ".zip") == 0 || strcasecmp(ext, ".j64") == 0
				|| strcasecmp(ext, ".abs") == 0 || strcasecmp(ext, ".jag") == 0
				|| strcasecmp(ext, ".rom") == 0)
				files->AddItem(string(de->d_name));
	}

	closedir(dp);
}

void FileList::HandleKey(SDLKey key)
{
	if (key == SDLK_RETURN)
		Notify(load);
	else
		Window::HandleKey(key);
}

void FileList::Notify(Element * e)
{
	if (e == load)
	{
		char filename[MAX_PATH];
		strcpy(filename, vjs.ROMPath);

		if (strlen(filename) > 0)
			if (filename[strlen(filename) - 1] != '/')
				strcat(filename, "/");

		strcat(filename, files->GetSelectedItem().c_str());

//		uint32_t romSize = JaguarLoadROM(jaguarMainROM, filename);
//		JaguarLoadCart(jaguarMainROM, filename);
		if (JaguarLoadFile(filename))
		{
			jaguarCartInserted = true;
			SDL_Event event;
			event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
			SDL_PushEvent(&event);

			event.type = SDL_USEREVENT, event.user.code = MENU_ITEM_CHOSEN;
			event.user.data1 = (void *)ResetJaguar;
	    	SDL_PushEvent(&event);
		}
		else
		{
			SDL_Event event;
			event.type = SDL_USEREVENT, event.user.code = WINDOW_CLOSE;
			SDL_PushEvent(&event);

			// Handle the error, but don't run...
			// Tell the user that we couldn't run their file for some reason... !!! FIX !!!
//how to kludge: Make a function like ResetJaguar which creates the dialog window
		}
	}
	else
		Window::Notify(e);
}


struct NameAction
{
	string name;
	Window * (* action)(void);
	SDLKey hotKey;

	NameAction(string n, Window * (* a)(void) = NULL, SDLKey k = SDLK_UNKNOWN): name(n),
		action(a), hotKey(k) {}
};


class MenuItems
{
	public:
		MenuItems(): charLength(0) {}
		bool Inside(uint32_t x, uint32_t y)
		{ return (x >= (uint32_t)extents.x && x < (uint32_t)(extents.x + extents.w)
		&& y >= (uint32_t)extents.y && y < (uint32_t)(extents.y + extents.h) ? true : false); }

		string title;
		vector<NameAction> item;
		uint32_t charLength;
		SDL_Rect extents;
};

class Menu: public Element
{
	public:
// 1CFF -> 0 001 11 00  111 1 1111 
// 421F -> 0 100 00 10  000 1 1111
		Menu(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 8,
/*			uint16_t fgc = 0x1CFF, uint16_t bgc = 0x000F, uint16_t fgch = 0x421F,
			uint16_t bgch = 0x1CFF): Element(x, y, w, h), activated(false), clicked(false),*/
/*			uint32_t fgc = 0xFF3F3F00, uint32_t bgc = 0x7F000000, uint32_t fgch = 0xFF878700,
			uint32_t bgch = 0xFF3F3F00): Element(x, y, w, h), activated(false), clicked(false),*/
			uint32_t fgc = 0xFFFF3F3F, uint32_t bgc = 0xFF7F0000, uint32_t fgch = 0xFFFF8787,
			uint32_t bgch = 0xFFFF3F3F): Element(x, y, w, h), activated(false), clicked(false),
			inside(0), insidePopup(0), fgColor(fgc), bgColor(bgc), fgColorHL(fgch),
			bgColorHL(bgch), menuChosen(-1), menuItemChosen(-1) {}
		virtual void HandleKey(SDLKey key);
		virtual void HandleMouseMove(uint32_t x, uint32_t y);
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown);
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0);
		virtual void Notify(Element *) {}
		void Add(MenuItems mi);

	protected:
		bool activated, clicked;
		uint32_t inside, insidePopup;
//		uint16_t fgColor, bgColor, fgColorHL, bgColorHL;
		uint32_t fgColor, bgColor, fgColorHL, bgColorHL;
		int menuChosen, menuItemChosen;

	private:
		vector<MenuItems> itemList;
};

void Menu::HandleKey(SDLKey key)
{
	for(uint32_t i=0; i<itemList.size(); i++)
	{
		for(uint32_t j=0; j<itemList[i].item.size(); j++)
		{
			if (itemList[i].item[j].hotKey == key)
			{
				SDL_Event event;
				event.type = SDL_USEREVENT;
				event.user.code = MENU_ITEM_CHOSEN;
				event.user.data1 = (void *)itemList[i].item[j].action;
	    		SDL_PushEvent(&event);

				clicked = false, menuChosen = menuItemChosen = -1;
				break;
			}
		}
	}
}

void Menu::HandleMouseMove(uint32_t x, uint32_t y)
{
	inside = insidePopup = 0;

	if (Inside(x, y))
	{
		// Find out *where* we are inside the menu bar
		uint32_t xpos = extents.x;

		for(uint32_t i=0; i<itemList.size(); i++)
		{
			uint32_t width = (itemList[i].title.length() + 2) * 8;

			if (x >= xpos && x < xpos + width)
			{
				inside = i + 1;
				menuChosen = i;
				break;
			}

			xpos += width;
		}
	}

	if (!Inside(x, y) && !clicked)
	{
		menuChosen = -1;
	}

	if (itemList[menuChosen].Inside(x, y) && clicked)
	{
		insidePopup = ((y - itemList[menuChosen].extents.y) / 8) + 1;
		menuItemChosen = insidePopup - 1;
	}
}

void Menu::HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown)
{
	if (!clicked)
	{
		if (mouseDown)
		{
			if (inside)
				clicked = true;
			else
				menuChosen = -1;					// clicked is already false...!
		}
	}
	else											// clicked == true
	{
		if (insidePopup && !mouseDown)				// I.e., mouse-button-up
		{
			activated = true;
			if (itemList[menuChosen].item[menuItemChosen].action != NULL)
			{
//				itemList[menuChosen].item[menuItemChosen].action();
				SDL_Event event;
				event.type = SDL_USEREVENT;
				event.user.code = MENU_ITEM_CHOSEN;
				event.user.data1 = (void *)itemList[menuChosen].item[menuItemChosen].action;
			    SDL_PushEvent(&event);

				clicked = false, menuChosen = menuItemChosen = -1;

/*				SDL_Event event;
				while (SDL_PollEvent(&event));		// Flush the event queue...
				event.type = SDL_MOUSEMOTION;
				int mx, my;
				SDL_GetMouseState(&mx, &my);
				event.motion.x = mx, event.motion.y = my;
			    SDL_PushEvent(&event);				// & update mouse position...!
*/			}
		}

		if (!inside && !insidePopup && mouseDown)
			clicked = false, menuChosen = menuItemChosen = -1;
	}
}

void Menu::Draw(uint32_t offsetX/*= 0*/, uint32_t offsetY/*= 0*/)
{
	uint32_t xpos = extents.x + offsetX;

	for(uint32_t i=0; i<itemList.size(); i++)
	{
//		uint16_t color1 = fgColor, color2 = bgColor;
		uint32_t color1 = fgColor, color2 = bgColor;
		if (inside == (i + 1) || (menuChosen != -1 && (uint32_t)menuChosen == i))
			color1 = fgColorHL, color2 = bgColorHL;

		DrawStringOpaque(screenBuffer, xpos, extents.y + offsetY, color1, color2,
			" %s ", itemList[i].title.c_str());
		xpos += (itemList[i].title.length() + 2) * 8;
	}

	// Draw sub menu (but only if active)
	if (clicked)
	{
		uint32_t ypos = extents.y + 9;

		for(uint32_t i=0; i<itemList[menuChosen].item.size(); i++)
		{
//			uint16_t color1 = fgColor, color2 = bgColor;
			uint32_t color1 = fgColor, color2 = bgColor;

			if (insidePopup == i + 1)
				color1 = fgColorHL, color2 = bgColorHL, menuItemChosen = i;

			if (itemList[menuChosen].item[i].name.length() > 0)
				DrawStringOpaque(screenBuffer, itemList[menuChosen].extents.x, ypos,
					color1, color2, " %-*.*s ", itemList[menuChosen].charLength,
					itemList[menuChosen].charLength, itemList[menuChosen].item[i].name.c_str());
			else
				DrawStringOpaque(screenBuffer, itemList[menuChosen].extents.x, ypos,
					fgColor, bgColor, "%.*s", itemList[menuChosen].charLength + 2, separator);

			ypos += 8;
		}
	}
}

void Menu::Add(MenuItems mi)
{
	for(uint32_t i=0; i<mi.item.size(); i++)
		if (mi.item[i].name.length() > mi.charLength)
			mi.charLength = mi.item[i].name.length();

	// Set extents here as well...
	mi.extents.x = extents.x + extents.w, mi.extents.y = extents.y + 9;
	mi.extents.w = (mi.charLength + 2) * 8, mi.extents.h = mi.item.size() * 8;

	itemList.push_back(mi);
	extents.w += (mi.title.length() + 2) * 8;
}


//Do we even *need* this?
//Doesn't seem like it...
/*class RootWindow: public Window
{
	public:
		RootWindow(Menu * m, Window * w = NULL): menu(m), window(w) {}
//Do we even need to care about this crap?
//			{ extents.x = extents.y = 0, extents.w = 320, extents.h = 240; }
		virtual void HandleKey(SDLKey key) {}
		virtual void HandleMouseMove(uint32_t x, uint32_t y) {}
		virtual void HandleMouseButton(uint32_t x, uint32_t y, bool mouseDown) {}
		virtual void Draw(uint32_t offsetX = 0, uint32_t offsetY = 0) {}
		virtual void Notify(Element *) {}

	private:
		Menu * menu;
		Window * window;
		int16_t * rootImage[1280 * 240 * 2];
};//*/


//
// Draw text at the given x/y coordinates. Can invert text as well.
//
//void DrawString(int16_t * screen, uint32_t x, uint32_t y, bool invert, const char * text, ...)
void DrawString(uint32_t * screen, uint32_t x, uint32_t y, bool invert, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

//	uint32_t pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32_t pitch = GetSDLScreenWidthInPixels();
	uint32_t length = strlen(string), address = x + (y * pitch);

	for(uint32_t i=0; i<length; i++)
	{
		uint32_t fontAddr = (uint32_t)string[i] * 64;

		for(uint32_t yy=0; yy<8; yy++)
		{
			for(uint32_t xx=0; xx<8; xx++)
			{
				if ((font1[fontAddr] && !invert) || (!font1[fontAddr] && invert))
//					*(screen + address + xx + (yy * pitch)) = 0xFE00;
					*(screen + address + xx + (yy * pitch)) = 0xFF0080FF;
				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// Draw text at the given x/y coordinates, using FG/BG colors.
//
//void DrawStringOpaque(int16_t * screen, uint32_t x, uint32_t y, uint16_t color1, uint16_t color2, const char * text, ...)
void DrawStringOpaque(uint32_t * screen, uint32_t x, uint32_t y, uint32_t color1, uint32_t color2, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

//	uint32_t pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32_t pitch = GetSDLScreenWidthInPixels();
	uint32_t length = strlen(string), address = x + (y * pitch);

	for(uint32_t i=0; i<length; i++)
	{
		uint32_t fontAddr = (uint32_t)string[i] * 64;

		for(uint32_t yy=0; yy<8; yy++)
		{
			for(uint32_t xx=0; xx<8; xx++)
			{
				*(screen + address + xx + (yy * pitch)) = (font1[fontAddr] ? color1 : color2);
				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// Draw text at the given x/y coordinates with transparency (0 is fully opaque, 32 is fully transparent).
//
//void DrawStringTrans(int16_t * screen, uint32_t x, uint32_t y, uint16_t color, uint8_t trans, const char * text, ...)
void DrawStringTrans(uint32_t * screen, uint32_t x, uint32_t y, uint32_t color, uint8_t trans, const char * text, ...)
{
	char string[4096];
	va_list arg;

	va_start(arg, text);
	vsprintf(string, text, arg);
	va_end(arg);

//	uint32_t pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32_t pitch = GetSDLScreenWidthInPixels();
	uint32_t length = strlen(string), address = x + (y * pitch);

	for(uint32_t i=0; i<length; i++)
	{
		uint32_t fontAddr = (uint32_t)string[i] * 64;

		for(uint32_t yy=0; yy<8; yy++)
		{
			for(uint32_t xx=0; xx<8; xx++)
			{
				if (font1[fontAddr])
				{
//					uint16_t existingColor = *(screen + address + xx + (yy * pitch));
					uint32_t existingColor = *(screen + address + xx + (yy * pitch));

/*					uint8_t eRed = (existingColor >> 10) & 0x1F,
						eGreen = (existingColor >> 5) & 0x1F,
						eBlue = existingColor & 0x1F,
//This could be done ahead of time, instead of on each pixel...
						nRed = (color >> 10) & 0x1F,
						nGreen = (color >> 5) & 0x1F,
						nBlue = color & 0x1F;//*/
					uint8_t eBlue = (existingColor >> 16) & 0xFF,
						eGreen = (existingColor >> 8) & 0xFF,
						eRed = existingColor & 0xFF,
//This could be done ahead of time, instead of on each pixel...
						nBlue = (color >> 16) & 0xFF,
						nGreen = (color >> 8) & 0xFF,
						nRed = color & 0xFF;

//This could be sped up by using a table of 5 + 5 + 5 bits (32 levels transparency -> 32768 entries)
//Here we've modified it to have 33 levels of transparency (could have any # we want!)
//because dividing by 32 is faster than dividing by 31...!
					uint8_t invTrans = 32 - trans;
/*					uint16_t bRed = (eRed * trans + nRed * invTrans) / 32;
					uint16_t bGreen = (eGreen * trans + nGreen * invTrans) / 32;
					uint16_t bBlue = (eBlue * trans + nBlue * invTrans) / 32;

					uint16_t blendedColor = (bRed << 10) | (bGreen << 5) | bBlue;//*/

					uint32_t bRed = (eRed * trans + nRed * invTrans) / 32;
					uint32_t bGreen = (eGreen * trans + nGreen * invTrans) / 32;
					uint32_t bBlue = (eBlue * trans + nBlue * invTrans) / 32;

					*(screen + address + xx + (yy * pitch)) = 0xFF000000 | (bBlue << 16) | (bGreen << 8) | bRed;
				}

				fontAddr++;
			}
		}

		address += 8;
	}
}

//
// Draw "picture"
// Uses zero as transparent color
// Can also use an optional alpha channel
//
//void DrawTransparentBitmap(int16_t * screen, uint32_t x, uint32_t y, uint16_t * bitmap, uint8_t * alpha/*=NULL*/)
void DrawTransparentBitmap(uint32_t * screen, uint32_t x, uint32_t y, uint32_t * bitmap, uint8_t * alpha/*=NULL*/)
{
	uint32_t width = bitmap[0], height = bitmap[1];
	bitmap += 2;

//	uint32_t pitch = GetSDLScreenPitch() / 2;			// Returns pitch in bytes but we need words...
	uint32_t pitch = GetSDLScreenWidthInPixels();
	uint32_t address = x + (y * pitch);

	for(uint32_t yy=0; yy<height; yy++)
	{
		for(uint32_t xx=0; xx<width; xx++)
		{
			if (alpha == NULL)
			{
				if (*bitmap && x + xx < pitch)			// NOTE: Still doesn't clip the Y val...
					*(screen + address + xx + (yy * pitch)) = *bitmap;
			}
			else
			{
				uint8_t trans = *alpha;
				uint32_t color = *bitmap;
				uint32_t existingColor = *(screen + address + xx + (yy * pitch));

				uint8_t eRed = existingColor & 0xFF,
					eGreen = (existingColor >> 8) & 0xFF,
					eBlue = (existingColor >> 16) & 0xFF,

					nRed = color & 0xFF,
					nGreen = (color >> 8) & 0xFF,
					nBlue = (color >> 16) & 0xFF;

				uint8_t invTrans = 255 - trans;
				uint32_t bRed = (eRed * trans + nRed * invTrans) / 255;
				uint32_t bGreen = (eGreen * trans + nGreen * invTrans) / 255;
				uint32_t bBlue = (eBlue * trans + nBlue * invTrans) / 255;

				uint32_t blendedColor = 0xFF000000 | bRed | (bGreen << 8) | (bBlue << 16);

				*(screen + address + xx + (yy * pitch)) = blendedColor;

				alpha++;
			}

			bitmap++;
		}
	}
}


//
// GUI stuff--it's not crunchy, it's GUI! ;-)
//

void GUIInit(void)
{
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GetMouseState(&mouseX, &mouseY);
}

void GUIDone(void)
{
}

//
// GUI main loop
//
//bool GUIMain(void)
bool GUIMain(char * filename)
{
WriteLog("GUI: Inside GUIMain...\n");
// Need to set things up so that it loads and runs a file if given on the command line. !!! FIX !!!
	extern uint32_t * backbuffer;
//	bool done = false;
	SDL_Event event;
	Window * mainWindow = NULL;

	// Set up the GUI classes...
//	Element::SetScreenAndPitch(backbuffer, GetSDLScreenPitch() / 2);
	Element::SetScreenAndPitch(backbuffer, GetSDLScreenWidthInPixels());

	Menu mainMenu;
	MenuItems mi;
	mi.title = "Jaguar";
	mi.item.push_back(NameAction("Load...", LoadROM, SDLK_l));
	mi.item.push_back(NameAction("Reset", ResetJaguar, SDLK_r));
//	if (CDBIOSLoaded)
//		mi.item.push_back(NameAction("Reset CD", ResetJaguarCD, SDLK_c));
	mi.item.push_back(NameAction("Run", RunEmu, SDLK_ESCAPE));
	mi.item.push_back(NameAction(""));
	mi.item.push_back(NameAction("Quit", Quit, SDLK_q));
	mainMenu.Add(mi);
	mi.title = "Settings";
	mi.item.clear();
	mi.item.push_back(NameAction("Video..."));
	mi.item.push_back(NameAction("Audio..."));
	mi.item.push_back(NameAction("Misc...", MiscOptions, SDLK_m));
	mainMenu.Add(mi);
	mi.title = "Info";
	mi.item.clear();
	mi.item.push_back(NameAction("About...", About));
	mainMenu.Add(mi);

	bool showMouse = true;

//This is crappy!!! !!! FIX !!!
//Is this even needed any more? Hmm. Maybe. Dunno.
WriteLog("GUI: Resetting Jaguar...\n");
	JaguarReset();

WriteLog("GUI: Clearing BG save...\n");
	// Set up our background save...
//	memset(background, 0x11, TOMGetVideoModeWidth() * 240 * 2);
//1111 -> 000100 01000 10001 -> 0001 0000 0100 0010 1000 1100 -> 10 42 8C
	for(uint32_t i=0; i<TOMGetVideoModeWidth()*240; i++)
		background[i] = 0xFF8C4210;

	// Handle loading file passed in on the command line...! [DONE]

	if (filename)
	{
		if (JaguarLoadFile(filename))
		{
			jaguarCartInserted = true;
//			event.type = SDL_USEREVENT, event.user.code = MENU_ITEM_CHOSEN;
//			event.user.data1 = (void *)ResetJaguar;
//	    	SDL_PushEvent(&event);
			// Make it so that if passed in on the command line, we quit right
			// away when pressing ESC
WriteLog("GUI: Bypassing GUI since ROM passed in on command line...\n");
			ResetJaguar();
			return true;
		}
		else
		{
			// Create error dialog...
			char errText[1024];
			sprintf(errText, "The file %40s could not be loaded.", filename);

			mainWindow = new Window(8, 16, 304, 160);
			mainWindow->AddElement(new Text(8, 8, "Error!"));
			mainWindow->AddElement(new Text(8, 24, errText));
		}
	}

WriteLog("GUI: Entering main loop...\n");
	while (!exitGUI)
	{
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_USEREVENT)
			{
				if (event.user.code == WINDOW_CLOSE)
				{
					delete mainWindow;
					mainWindow = NULL;
				}
				else if (event.user.code == MENU_ITEM_CHOSEN)
				{
					// Confused? Let me enlighten... What we're doing here is casting
					// data1 as a pointer to a function which returns a Window pointer and
					// which takes no parameters (the "(Window *(*)(void))" part), then
					// derefencing it (the "*" in front of that) in order to call the
					// function that it points to. Clear as mud? Yeah, I hate function
					// pointers too, but what else are you gonna do?
					mainWindow = (*(Window *(*)(void))event.user.data1)();

					while (SDL_PollEvent(&event));	// Flush the event queue...
					event.type = SDL_MOUSEMOTION;
					int mx, my;
					SDL_GetMouseState(&mx, &my);
					event.motion.x = mx, event.motion.y = my;
				    SDL_PushEvent(&event);			// & update mouse position...!

					mouseX = mx, mouseY = my;		// This prevents "mouse flash"...
					if (vjs.useOpenGL)
						mouseX /= 2, mouseY /= 2;
				}
			}
			else if (event.type == SDL_ACTIVEEVENT)
			{
				if (event.active.state == SDL_APPMOUSEFOCUS)
					showMouse = (event.active.gain ? true : false);
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if (mainWindow)
					mainWindow->HandleKey(event.key.keysym.sym);
				else
					mainMenu.HandleKey(event.key.keysym.sym);
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				mouseX = event.motion.x, mouseY = event.motion.y;

				if (vjs.useOpenGL)
					mouseX /= 2, mouseY /= 2;

				if (mainWindow)
					mainWindow->HandleMouseMove(mouseX, mouseY);
				else
					mainMenu.HandleMouseMove(mouseX, mouseY);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				uint32_t mx = event.button.x, my = event.button.y;

				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				if (mainWindow)
					mainWindow->HandleMouseButton(mx, my, true);
				else
					mainMenu.HandleMouseButton(mx, my, true);
			}
			else if (event.type == SDL_MOUSEBUTTONUP)
			{
				uint32_t mx = event.button.x, my = event.button.y;

				if (vjs.useOpenGL)
					mx /= 2, my /= 2;

				if (mainWindow)
					mainWindow->HandleMouseButton(mx, my, false);
				else
					mainMenu.HandleMouseButton(mx, my, false);
			}

			// Draw the GUI...
// The way we do things here is kinda stupid (redrawing the screen every frame), but
// it's simple. Perhaps there may be a reason down the road to be more selective with
// our clearing, but for now, this will suffice.
//			memset(backbuffer, 0x11, TOMGetVideoModeWidth() * 240 * 2);
//			memcpy(backbuffer, background, TOMGetVideoModeWidth() * 256 * 2);
			memcpy(backbuffer, background, TOMGetVideoModeWidth() * 256 * 4);

			mainMenu.Draw();
//Could do multiple windows here by using a vector + priority info...
//Though the way ZSNES does it seems to be by a bool (i.e., they're always active, just not shown)
			if (mainWindow)
				mainWindow->Draw();

			if (showMouse)
//				DrawTransparentBitmap(backbuffer, mouseX, mouseY, mousePic);
				DrawTransparentBitmap(backbuffer, mouseX, mouseY, mousePic);

			RenderBackbuffer();
		}
	}

	return true;
}

//
// GUI "action" functions
//

Window * LoadROM(void)
{
	FileList * fileList = new FileList(8, 16, 304, 216);

	return (Window *)fileList;
}

Window * ResetJaguar(void)
{
	JaguarReset();

	return RunEmu();
}

Window * ResetJaguarCD(void)
{
//	memcpy(jaguarMainROM, jaguarCDBootROM, 0x40000);
//	jaguarRunAddress = 0x802000;
//	jaguarMainROMCRC32 = crc32_calcCheckSum(jaguarMainROM, 0x40000);
	JaguarReset();
//This is a quick kludge to get the CDBIOS to boot properly...
//Wild speculation: It could be that this memory location is wired into the CD unit
//somehow, which lets it know whether or not a cart is present in the unit...
//	jaguarMainROM[0x0040B] = 0x03;

	return RunEmu();
}

bool debounceRunKey = true;
Window * RunEmu(void)
{
//This is crappy... !!! FIX !!!
	extern uint32_t * backbuffer;
	extern bool finished, showGUI;

	uint32_t nFrame = 0, nFrameskip = 0;
	uint32_t totalFrames = 0;
	finished = false;
	bool showMessage = true;
	uint32_t showMsgFrames = 120;
	uint8_t transparency = 0;
	// Pass a message to the "joystick" code to debounce the ESC key...
	debounceRunKey = true;

	uint32_t cartType = 4;
	if (jaguarROMSize == 0x200000)
		cartType = 0;
	else if (jaguarROMSize == 0x400000)
		cartType = 1;
	else if (jaguarMainROMCRC32 == 0x687068D5)
		cartType = 2;
	else if (jaguarMainROMCRC32 == 0x55A0669C)
		cartType = 3;

	char * cartTypeName[5] = { "2M Cartridge", "4M Cartridge", "CD BIOS", "CD Dev BIOS", "Homebrew" };

//	while (!finished)
	while (true)
	{
		// Set up new backbuffer with new pixels and data
//		JaguarExecute(backbuffer, true);
	  JaguarExecuteNew();
		totalFrames++;
//WriteLog("Frame #%u...\n", totalFrames);
//extern bool doDSPDis;
//if (totalFrames == 373)
//	doDSPDis = true;

//This sucks... !!! FIX !!!
		JoystickExec();
//This is done here so that the crud below doesn't get on our GUI background...
		if (finished)
			break;

		// Some QnD GUI stuff here...
		if (showGUI)
		{
			extern uint32_t gpu_pc, dsp_pc;
			DrawString((uint32_t *)backbuffer, 8, 8, false, "GPU PC: %08X", gpu_pc);
			DrawString((uint32_t *)backbuffer, 8, 16, false, "DSP PC: %08X", dsp_pc);
		}

		if (showMessage)
		{
// FF0F -> 1111 11 11  000 0 1111 -> 3F 18 0F
// 3FE3 -> 0011 11 11  111 0 0011 -> 0F 3F 03
/*			DrawStringTrans((uint32_t *)backbuffer, 8, 24*8, 0xFF0F, transparency, "Running...");
			DrawStringTrans((uint32_t *)backbuffer, 8, 26*8, 0x3FE3, transparency, "%s, run address: %06X", cartTypeName[cartType], jaguarRunAddress);
			DrawStringTrans((uint32_t *)backbuffer, 8, 27*8, 0x3FE3, transparency, "CRC: %08X", jaguarMainROMCRC32);//*/
//first has wrong color. !!! FIX !!!
			DrawStringTrans((uint32_t *)backbuffer, 8, 24*8, 0xFF7F63FF, transparency, "Running...");
			DrawStringTrans((uint32_t *)backbuffer, 8, 26*8, 0xFF1FFF3F, transparency, "%s, run address: %06X", cartTypeName[cartType], jaguarRunAddress);
			DrawStringTrans((uint32_t *)backbuffer, 8, 27*8, 0xFF1FFF3F, transparency, "CRC: %08X", jaguarMainROMCRC32);

			if (showMsgFrames == 0)
			{			
				transparency++;

				if (transparency == 33)
{
					showMessage = false;
/*extern bool doGPUDis;
doGPUDis = true;//*/
}

			}
			else
				showMsgFrames--;
		}

		// Simple frameskip
		if (nFrame == nFrameskip)
		{
			RenderBackbuffer();
			nFrame = 0;
		}
		else
			nFrame++;
	}

	// Reset the pitch, since it may have been changed in-game...
//	Element::SetScreenAndPitch(backbuffer, GetSDLScreenPitch() / 2);
	Element::SetScreenAndPitch((uint32_t *)backbuffer, GetSDLScreenWidthInPixels());

	// Save the background for the GUI...
//	memcpy(background, backbuffer, TOMGetVideoModeWidth() * 240 * 2);
	// In this case, we squash the color to monochrome, then force it to blue + green...
	for(uint32_t i=0; i<TOMGetVideoModeWidth() * 256; i++)
	{
//		uint32_t word = backbuffer[i];
//		uint8_t r = (word >> 10) & 0x1F, g = (word >> 5) & 0x1F, b = word & 0x1F;
//		word = ((r + g + b) / 3) & 0x001F;
//		word = (word << 5) | word;
//		background[i] = word;

		uint32_t pixel = backbuffer[i];
		uint8_t b = (pixel >> 16) & 0xFF, g = (pixel >> 8) & 0xFF, r = pixel & 0xFF;
		pixel = ((r + g + b) / 3) & 0x00FF;
		background[i] = 0xFF000000 | (pixel << 16) | (pixel << 8);
	}

	return NULL;
}

Window * Quit(void)
{
	WriteLog("GUI: Quitting due to user request.\n");
	exitGUI = true;

	return NULL;
}

Window * About(void)
{
	Window * window = new Window(8, 16, 304, 160);
//	window->AddElement(new Text(8, 8, "Virtual Jaguar 1.0.8"));
	window->AddElement(new Text(8, 8, "Virtual Jaguar CVS 20041224"));
	window->AddElement(new Text(8, 24, "Coders:"));
	window->AddElement(new Text(16, 32, "Niels Wagenaar (nwagenaar)"));
	window->AddElement(new Text(16, 40, "Carwin Jones (Caz)"));
	window->AddElement(new Text(16, 48, "James L. Hammons (shamus)"));
	window->AddElement(new Text(16, 56, "Adam Green"));
	window->AddElement(new Text(8, 72, "Testers:"));
	window->AddElement(new Text(16, 80, "Guruma"));
	window->AddElement(new Text(8, 96, "Thanks go out to:"));
	window->AddElement(new Text(16, 104, "Aaron Giles (original CoJag)"));
	window->AddElement(new Text(16, 112, "David Raingeard (original VJ)"));
	window->AddElement(new Text(16, 120, "Karl Stenerud (Musashi 68K emu)"));
	window->AddElement(new Text(16, 128, "Sam Lantinga (amazing SDL libs)"));
	window->AddElement(new Text(16, 136, "Ryan C. Gordon (VJ's web presence)"));
	window->AddElement(new Text(16, 144, "The guys over at Atari Age ;-)"));

	return window;
}

Window * MiscOptions(void)
{
	Window * window = new Window(8, 16, 304, 160);
	window->AddElement(new PushButton(8, 8, &vjs.useJaguarBIOS, "BIOS"));
	window->AddElement(new SlideSwitch(8, 20, &vjs.hardwareTypeNTSC, "PAL", "NTSC"));
	window->AddElement(new PushButton(8, 40, &vjs.DSPEnabled, "DSP"));
	window->AddElement(new SlideSwitch(16, 52, &vjs.usePipelinedDSP, "Original", "Pipelined"));
	window->AddElement(new SlideSwitch(8, 72, (bool *)&vjs.glFilter, "Sharp", "Blurry"));

// Missing:
// * BIOS path
// * ROM path
// * EEPROM path
// * joystick
// * joystick port
// * OpenGL?
// * GL Filter type
// * Window/fullscreen

	return window;
}

// Function prototype
Window * CrashGracefullyCallback(void);

//NOTE: Probably should set a flag as well telling it to do a full reset
//      of the Jaguar hardware if this happens...
void GUICrashGracefully(const char * reason)
{
	finished = true;							// We're finished for now!

	// Since this is used in the menu code as well, we could create another
	// internal function called "PushWindowOnQueue" or somesuch
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = MENU_ITEM_CHOSEN;
	event.user.data1 = (void *)CrashGracefullyCallback;
	SDL_PushEvent(&event);
}

Window * CrashGracefullyCallback(void)
{
	Window * window = new Window(8, 16, 304, 192);

	window->AddElement(new Text(8, 8+0*FONT_HEIGHT, "We CRASHED!!!"));

	return window;
}