//
// VIDEO.H: Header file
//

#ifndef __VIDEO_H__
#define __VIDEO_H__

bool VideoInit(void);
void VideoDone(void);
void RenderBackbuffer(void);
void ResizeScreen(uint32_t width, uint32_t height);
uint32_t GetSDLScreenPitch(void);
uint32_t GetSDLScreenWidthInPixels(void);
void ToggleFullscreen(void);

extern uint32_t * backbuffer;

#endif	// __VIDEO_H__
