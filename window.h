#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <libtcod.hpp>
#include <vector>
#include "glyph.h"

//      LINE_NESW  - X for on, O for off
#define LINE_XOXO 4194424
#define LINE_OXOX 4194417
#define LINE_XXOO 4194413
#define LINE_OXXO 4194412
#define LINE_OOXX 4194411
#define LINE_XOOX 4194410
#define LINE_XXXO 4194420
#define LINE_XXOX 4194422
#define LINE_XOXX 4194421
#define LINE_OXXX 4194423
#define LINE_XXXX 4194414

#define KEY_ESC 27

void init_display();
long input();
void debugmsg(const char *mes, ...);
void refresh_all(bool erase = false);

std::string key_name(long ch);

std::string file_selector(std::string start = ".");

std::string string_edit_popup(std::string orig, const char *mes, ...);
std::string string_input_popup(const char *mes, ...);
int         int_input_popup(const char *mes, ...);
long popup_getkey(const char *mes, ...);
int  menu_vec(const char *mes, std::vector<std::string> options);
int  menu(const char *mes, ...);
void popup(const char *mes, ...);
void popup_fullscreen(const char *mes, ...);

class Window
{
 public:
  Window();
  Window(int posx, int posy, int sizex, int sizey);
  ~Window();
  void init(int posx, int posy, int sizex, int sizey);
  void close();

  void outline();

// Info functions
  glyph glyphat(int x, int y);
// The three essential output functions
  void putch(int x, int y, nc_color fg, nc_color, long sym);
  void putglyph(int x, int y, glyph gl);
  void putstr(int x, int y, nc_color fg, nc_color bg, std::string str, ...);
  void putstr_raw(int x, int y, nc_color fg, nc_color bg, std::string str, ...);
  void putstr_n(int x, int y, nc_color fg, nc_color bg, int maxlength,
                std::string str, ...);
// Special helper drawing functions
  void clear_area(int x1, int y1, int x2, int y2);
  void line_v(int x, nc_color fg = c_white, nc_color bg = c_black);
  void line_h(int y, nc_color fg = c_white, nc_color bg = c_black);

  void clear();
  void refresh();
 private:
  TCODConsole* w;
  int m_posx, m_posy;
  bool outlined;
  int xdim, ydim;
};

#endif
