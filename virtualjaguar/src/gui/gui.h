//
// GUI.H
//
// Graphical User Interface support
//

#ifndef __GUI_H__
#define __GUI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FONT_WIDTH		8
#define FONT_HEIGHT		16
  
void GUIInit(void);
void GUIDone(void);
//void DrawString(int16 * screen, uint32 x, uint32 y, bool invert, const char * text, ...);
//bool GUIMain(void);
bool GUIMain(char *);
void GUICrashGracefully(const char *);

extern bool showGUI;
extern bool exitGUI;	
extern bool finished;							// Current emulator loop is finished

#ifdef __cplusplus
}
#endif

#endif	// __GUI_H__
