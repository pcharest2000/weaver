#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include "weaveImage.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <iostream>
#include <memory>
#include <sdlgui/button.h>
#include <sdlgui/checkbox.h>
#include <sdlgui/colorwheel.h>
#include <sdlgui/combobox.h>
#include <sdlgui/dropdownbox.h>
#include <sdlgui/entypo.h>
#include <sdlgui/formhelper.h>
#include <sdlgui/graph.h>
#include <sdlgui/imagepanel.h>
#include <sdlgui/imageview.h>
#include <sdlgui/label.h>
#include <sdlgui/layout.h>
#include <sdlgui/messagedialog.h>
#include <sdlgui/popupbutton.h>
#include <sdlgui/progressbar.h>
#include <sdlgui/screen.h>
#include <sdlgui/slider.h>
#include <sdlgui/switchbox.h>
#include <sdlgui/tabwidget.h>
#include <sdlgui/textbox.h>
#include <sdlgui/toolbutton.h>
#include <sdlgui/vscrollpanel.h>
#include <sdlgui/window.h>
#include <stdio.h>
using std::cerr;
using std::cout;
using std::endl;
using namespace sdlgui;
class MainWindow : public Screen {
public:
  MainWindow(SDL_Window *_win, WeaveImage *weaver, int width, int heigth);
  int _width;
  int _heigth;

private:
  SDL_Window *_win = NULL;
  SDL_Renderer *_renderer = NULL;
  WeaveImage *_weaver;
};

#endif // __MAINWINDOW_H_
