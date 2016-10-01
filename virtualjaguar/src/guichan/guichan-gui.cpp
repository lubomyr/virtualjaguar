#include <algorithm>
#ifdef ANDROID
#include <android/log.h>
#endif
#include <guichan.hpp>
#include <iostream>
#include <sstream>
#include <SDL/SDL_ttf.h>
#include <guichan/sdl.hpp>
#include "sdltruetypefont.hpp"
#include <unistd.h>

#define PANDORA

#if defined(ANDROID)
#include <SDL_screenkeyboard.h>
#endif

SDL_Surface* GUI_screen;
int emulating;
bool running = false;
int rungame=0;
bool menu_onscreen = true;

#define GUI_WIDTH  640
#define GUI_HEIGHT 400

/* What is being loaded */
#define MENU_SELECT_FILE 1
#define MENU_ADD_DIR 2

char currentDir[300];
char launchDir[300];

namespace globals
{
gcn::Gui* gui;
}

namespace sdl
{
// Main objects to draw graphics and get user input
gcn::SDLGraphics* graphics;
gcn::SDLInput* input;
gcn::SDLImageLoader* imageLoader;

void init()
{
#ifdef PANDORA
    char layersize[20];
    snprintf(layersize, 20, "%dx%d", GUI_WIDTH, GUI_HEIGHT);

    char bordercut[20];
    snprintf(bordercut, 20, "0,0,0,0");

#endif
    GUI_screen = SDL_SetVideoMode(GUI_WIDTH, GUI_HEIGHT, 16, SDL_SWSURFACE);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

#ifdef PANDORA
    SDL_ShowCursor(SDL_ENABLE);
#endif

    imageLoader = new gcn::SDLImageLoader();
    gcn::Image::setImageLoader(imageLoader);

    graphics = new gcn::SDLGraphics();
    graphics->setTarget(GUI_screen);

    input = new gcn::SDLInput();

    globals::gui = new gcn::Gui();
    globals::gui->setGraphics(graphics);
    globals::gui->setInput(input);
}


void halt()
{
    delete globals::gui;
    delete imageLoader;
    delete input;
    delete graphics;
}


void run()
{
    // The main loop
    while(running) {
        // Check user input
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                break;
            } else if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_F1:
                    running = false;
                    break;

#ifndef PANDORA
                case SDLK_f:
                    if (event.key.keysym.mod & KMOD_CTRL) {
                        // Works with X11 only
                        SDL_WM_ToggleFullScreen(GUI_screen);
                    }
                    break;
#endif
                }
            }
            input->pushInput(event);
        }
        // Now we let the Gui object perform its logic.
        globals::gui->logic();
        // Now we let the Gui object draw itself.
        globals::gui->draw();
        // Finally we update the screen.
        SDL_Flip(GUI_screen);
    }
}

}


namespace widgets
{
extern void run_menuLoad_guichan(char *curr_path, int aLoadType);
extern gcn::Window *window_load;
extern gcn::Window *window_warning;
extern gcn::Window *window_savestate;
void show_settings(void);
void menuSaveState_Init(void);
void menuSaveState_Exit(void);
void loadMenu_Init(void);
void loadMenu_Exit(void);
void menuMessage_Init(void);
void menuMessage_Exit(void);
std::string selectedFilePath;

gcn::Color baseCol;
gcn::Color baseColLabel;
gcn::Container* top;
gcn::contrib::SDLTrueTypeFont* font;

// Presets

// Main buttons
gcn::Image *background_image;
gcn::Icon* background;
gcn::Button* button_reset;
gcn::Button* button_resume;
gcn::Button* button_quit;
gcn::Button* button_savestate;
gcn::Container* backgrd_onscreen;
gcn::CheckBox* checkBox_onscreen;

class QuitButtonActionListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent& actionEvent) {
        running = false;

        exit(0);
    }
};
QuitButtonActionListener* quitButtonActionListener;


class ResetButtonActionListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent& actionEvent) {
        run_menuLoad_guichan(currentDir, MENU_SELECT_FILE);
    }
};
ResetButtonActionListener* resetButtonActionListener;


class ResumeButtonActionListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent& actionEvent) {
        running = false;
    }
};
ResumeButtonActionListener* resumeButtonActionListener;

class StateButtonActionListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent& actionEvent) {
        window_savestate->setVisible(true);
    }
};
StateButtonActionListener* stateButtonActionListener;

class OnscreenActionListener : public gcn::ActionListener
{
public:
    void action(const gcn::ActionEvent& actionEvent) {
        if (actionEvent.getSource() == checkBox_onscreen)
            if (checkBox_onscreen->isSelected())
                menu_onscreen=true;
            else
                menu_onscreen=false;
    }
};
OnscreenActionListener* onscreenActionListener;

void init()
{
    baseCol.r = 192;
    baseCol.g = 192;
    baseCol.b = 208;
    baseColLabel.r = baseCol.r;
    baseColLabel.g = baseCol.g;
    baseColLabel.b = baseCol.b;
    baseColLabel.a = 192;

    top = new gcn::Container();
    top->setDimension(gcn::Rectangle(0, 0, 640, 400));
    top->setBaseColor(baseCol);
    globals::gui->setTop(top);

    TTF_Init();
    font = new gcn::contrib::SDLTrueTypeFont("data/FreeSans.ttf", 17);
    gcn::Widget::setGlobalFont(font);

    background_image = gcn::Image::load("data/background.jpg");
    background = new gcn::Icon(background_image);

    //--------------------------------------------------
    // Create main buttons
    //--------------------------------------------------

    button_quit = new gcn::Button("Quit");
    button_quit->setSize(90,50);
    button_quit->setBaseColor(baseCol);
    button_quit->setId("Quit");
    quitButtonActionListener = new QuitButtonActionListener();
    button_quit->addActionListener(quitButtonActionListener);

    button_reset = new gcn::Button("Run");
    button_reset->setSize(90,50);
    button_reset->setBaseColor(baseCol);
    button_reset->setId("Reset");
    resetButtonActionListener = new ResetButtonActionListener();
    button_reset->addActionListener(resetButtonActionListener);

    button_resume = new gcn::Button("Resume");
    button_resume->setSize(90,50);
    button_resume->setBaseColor(baseCol);
    button_resume->setId("Resume");
    resumeButtonActionListener = new ResumeButtonActionListener();
    button_resume->addActionListener(resumeButtonActionListener);

    button_savestate = new gcn::Button("SaveStates");
    button_savestate->setSize(100,50);
    button_savestate->setBaseColor(baseCol);
    button_savestate->setId("SaveStates");
    stateButtonActionListener = new StateButtonActionListener();
    button_savestate->addActionListener(stateButtonActionListener);

    checkBox_onscreen = new gcn::CheckBox("On-Screen Control");
    checkBox_onscreen->setPosition(4,2);
    checkBox_onscreen->setId("onScrCtrl");
    checkBox_onscreen->setBaseColor(baseColLabel);
    onscreenActionListener = new OnscreenActionListener();
    checkBox_onscreen->addActionListener(onscreenActionListener);

    backgrd_onscreen = new gcn::Container();
    backgrd_onscreen->setOpaque(true);
    backgrd_onscreen->setBaseColor(baseColLabel);
    backgrd_onscreen->setPosition(20, 270);
    backgrd_onscreen->setSize(190, 27);
    backgrd_onscreen->add(checkBox_onscreen);
    backgrd_onscreen->setVisible(true);

    menuSaveState_Init();
    menuMessage_Init();
    loadMenu_Init();

    //--------------------------------------------------
    // Place everything on main form
    //--------------------------------------------------
    top->add(background, 0, 0);
    top->add(button_reset, 210, 330);
//    top->add(button_resume, 320, 330);
    top->add(button_quit, 430, 330);
//    top->add(button_savestate, 20, 330);
    top->add(backgrd_onscreen);
    top->add(window_savestate);
    top->add(window_load, 120, 20);
    top->add(window_warning, 170, 180);

    show_settings();
}

//--------------------------------------------------
// Initialize focus handling
//--------------------------------------------------
//    button_reset->setFocusable(true);
//    button_resume->setFocusable(true);
//    button_quit->setFocusable(true);

void halt()
{
    menuSaveState_Exit();
    menuMessage_Exit();
    loadMenu_Exit();

    delete button_reset;
    delete button_resume;
    delete button_quit;
    delete button_savestate;
    delete backgrd_onscreen;
    delete checkBox_onscreen;

    delete resumeButtonActionListener;
    delete resetButtonActionListener;
    delete quitButtonActionListener;
    delete stateButtonActionListener;
    delete onscreenActionListener;

    delete background;
    delete background_image;

    delete font;
    delete top;

}

//-----------------------------------------------
// Start of helper functions
//-----------------------------------------------
void show_settings()
{
    if (menu_onscreen==true)
        checkBox_onscreen->setSelected(true);
    else
        checkBox_onscreen->setSelected(false);
}

}

void guichan_gui()
{
    getcwd(launchDir, 250);
    strcpy(currentDir,".");

#ifdef ANDROID
    SDL_ANDROID_SetScreenKeyboardShown(0);
#endif
    running = true;

    sdl::init();
    widgets::init();
    sdl::run();
    widgets::halt();
    sdl::halt();
#ifdef ANDROID
    if (menu_onscreen)
        SDL_ANDROID_SetScreenKeyboardShown(1);
#endif

}
