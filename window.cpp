#include <vector>
#include <string>
#include <list>
#include <cstring>
#include <sstream>
#include "window.h"
#include "globals.h"

bool parse_color_tags(std::string text, std::vector<std::string> &segments,
                      std::vector<long> &color_pairs, nc_color fg = c_white,
                      nc_color bg = c_black);

TCODColor translateColor(int i)
{
 switch(i)
 {
  case c_black:   return TCODColor::black;
  case c_white:   return TCODColor::white;
  case c_red:     return TCODColor::darkRed;
  case c_green:   return TCODColor::darkGreen;
  case c_blue:    return TCODColor::darkBlue;
  case c_cyan:    return TCODColor::darkCyan;
  case c_magenta: return TCODColor::darkMagenta;
  case c_yellow:  return TCODColor::darkYellow;

  case c_ltgray:  return TCODColor::lightGrey;
  case c_dkgray:  return TCODColor::darkGrey;
  case c_ltred:   return TCODColor::lightRed;
  case c_ltgreen: return TCODColor::lightGreen;
  case c_ltblue:  return TCODColor::lightBlue;
  case c_ltcyan:  return TCODColor::lightCyan;
  case c_pink:    return TCODColor::lightPink;
  default: break;
 }

 return TCODColor::black;
}

Window::Window()
{
 w = new TCODConsole(0,0);
 w->setDefaultBackground(TCODColor::black);
 w->setDefaultForeground(TCODColor::lightGrey);
 clear();
 outlined = false;
 xdim = 0;
 ydim = 0;
 WINDOWLIST.push_back(this);
}

Window::Window(int posx, int posy, int sizex, int sizey) :
 m_posx(posx),
 m_posy(posy)
{
 w = new TCODConsole(sizex, sizey);
 w->setDefaultBackground(TCODColor::black);
 w->setDefaultForeground(TCODColor::lightGrey);
 clear();
 outlined = false;
 xdim = sizex;
 ydim = sizey;
 WINDOWLIST.push_back(this);
}

Window::~Window()
{
 delete w;
 w = 0;
 WINDOWLIST.remove(this);
}

void Window::init(int posx, int posy, int sizex, int sizey)
{
 delete w;
 m_posx = posx;
 m_posy = posy;
 w = new TCODConsole(sizex, sizey);
 w->setDefaultBackground(TCODColor::black);
 w->setDefaultForeground(TCODColor::lightGrey);
 clear();
 xdim = sizex;
 ydim = sizey;
}

void Window::close()
{
 delete w;
 w = 0;
 WINDOWLIST.remove(this);
 refresh_all(true);
}

void Window::outline()
{
 outlined = true;
 long col = get_color_pair(c_white, c_black);
 /*
 wattron(w, col);
 wborder(w, LINE_XOXO, LINE_XOXO, LINE_OXOX, LINE_OXOX,
            LINE_OXXO, LINE_OOXX, LINE_XXOO, LINE_XOOX );
 wattroff(w, col);
 */
}

glyph Window::glyphat(int x, int y)
{
 glyph ret;
 if (x < 0 || x >= xdim || y < 0 || y >= ydim)
  return ret; // Whatever a default glyph is

 //long wi = mvwinch(w, y, x);
 //ret.symbol = wi - ((wi & A_COLOR) + (wi & A_ATTRIBUTES));
 //ret.symbol = wi & A_CHARTEXT;
 //extract_colors(wi & A_COLOR, wi & A_ATTRIBUTES, ret.fg, ret.bg);
 ret.symbol = w->getChar(x,y);
 return ret;
}
 
void Window::putch(int x, int y, nc_color fg, nc_color bg, long sym)
{
/*
 if (outlined) {
  x++;
  y++;
 }
*/
 long col = get_color_pair(fg, bg);

 w->setBackgroundFlag(TCOD_BKGND_SET);
 w->setDefaultForeground( translateColor(fg) );
 w->setDefaultBackground( translateColor(bg) );

 int charcode=sym;
 if (sym > 255)
 {
  switch (sym) {
   case LINE_XOXO:
    charcode=179;
    break;
   case LINE_OXOX:
    charcode=196;
    break;
   case LINE_XXOO:
    charcode=192;
    break;
   case LINE_OXXO:
    charcode=218;
    break;
   case LINE_OOXX:
    charcode=191;
    break;
   case LINE_XOOX:
    charcode=217;
    break;
   case LINE_XXOX:
    charcode=193;
    break;
   case LINE_XXXO:
    charcode=195;
    break;
   case LINE_XOXX:
    charcode=180;
    break;
   case LINE_OXXX:
    charcode=194;
    break;
   case LINE_XXXX:
    charcode=197;
    break;
   default:
    break;
  }
 }
 if (charcode < 256)
  w->setChar(x, y, charcode);
 else
  printf("Window::putch %d %c\n", charcode, charcode);
}

void Window::putglyph(int x, int y, glyph gl)
{
 putch(x, y, gl.fg, gl.bg, gl.symbol);
}

void Window::putstr(int x, int y, nc_color fg, nc_color bg, std::string str,
                    ...)
{
 va_list ap;
 va_start(ap, str);
 char buff[8192];
 vsprintf(buff, str.c_str(), ap);
 va_end(ap);

 std::string prepped = buff;
 long col = get_color_pair(fg, bg);

 w->setBackgroundFlag(TCOD_BKGND_SET);
 w->setDefaultForeground( translateColor(fg) );
 w->setDefaultBackground( translateColor(bg) );
 if (prepped.find("<c=") == std::string::npos) {
// No need to do color segments, so just print!
  w->print(x,y,buff);
 } else { // We need to do color segments!
  std::vector<std::string> segments;
  std::vector<long> color_pairs;
  parse_color_tags(prepped, segments, color_pairs, fg, bg);
  int pos = 0;
  nc_color tfg, tbg;
  for (int i = 0; i < segments.size(); i++) {
   int col = color_pairs[i]&~(A_BOLD|A_BLINK);
   int attr= color_pairs[i]&(A_BOLD|A_BLINK);
   extract_colors(col, attr, tfg, tbg);
   w->setDefaultForeground( translateColor(tfg) );
   w->setDefaultBackground( translateColor(tbg) );
   w->print(x+pos,y, segments[i].c_str());
   pos += segments[i].length();
  }
 }        // We need to do color segments!

}

void Window::putstr_raw(int x, int y, nc_color fg, nc_color bg, std::string str,
                    ...)
{
 va_list ap;
 va_start(ap, str);
 char buff[8192];
 vsprintf(buff, str.c_str(), ap);
 va_end(ap);

 std::string prepped = buff;
 long col = get_color_pair(fg, bg);

 w->setBackgroundFlag(TCOD_BKGND_SET);
 w->setDefaultForeground( translateColor(fg) );
 w->setDefaultBackground( translateColor(bg) );
 w->print(x,y,buff);
}
 
void Window::putstr_n(int x, int y, nc_color fg, nc_color bg, int maxlength,
                      std::string str, ...)
{
 va_list ap;
 va_start(ap, str);
 char buff[8192];
 vsprintf(buff, str.c_str(), ap);
 va_end(ap);

 std::string prepped = buff;
 long col = get_color_pair(fg, bg);

 w->setBackgroundFlag(TCOD_BKGND_SET);
 w->setDefaultForeground( translateColor(fg) );
 w->setDefaultBackground( translateColor(bg) );
 if (prepped.find("<c=") == std::string::npos) {
// No need to do color segments, so just print!
  w->print(x,y, prepped.substr(0, maxlength).c_str());
 } else { // We need to do color segments!
  std::vector<std::string> segments;
  std::vector<long> color_pairs;
  parse_color_tags(prepped, segments, color_pairs, fg, bg);
  int pos=0;
  nc_color tfg, tbg;
  for (int i = 0; i < segments.size(); i++) {
   int col = color_pairs[i]&~(A_BOLD|A_BLINK);
   int attr= color_pairs[i]&(A_BOLD|A_BLINK);
   extract_colors(col, attr, tfg, tbg);
   w->setDefaultForeground( translateColor(tfg) );
   w->setDefaultBackground( translateColor(tbg) );
   if (segments[i].length() > maxlength) {
    w->print(x+pos, y, segments[i].substr(0, maxlength).c_str());
    return; // Stop; we've run out of space.
   } else {
    maxlength -= segments[i].length();
    w->print(x+pos, y, segments[i].substr(0, maxlength).c_str());
    pos += segments[i].length();
   }
  }
 } // We need to do color segments!

}

void Window::clear_area(int x1, int y1, int x2, int y2)
{
 for (int x = x1; x <= x2; x++) {
  for (int y = y1; y <= y2; y++)
   putch(x, y, c_black, c_black, 'x');
 }
}

void Window::line_v(int x, nc_color fg, nc_color bg)
{
 for (int y = (outlined ? 1 : 0); y < (outlined ? ydim - 1 : ydim); y++)
  putch(x, y, fg, bg, LINE_XOXO);

 if (outlined) { // Alter the outline so it attaches to our line
  putch(x, 0, fg, bg, LINE_OXXX);
  putch(x, ydim - 1, fg, bg, LINE_XXOX);
 }
}

void Window::line_h(int y, nc_color fg, nc_color bg)
{
 for (int x = (outlined ? 1 : 0); x < (outlined ? xdim - 1 : xdim); x++)
  putch(x, y, fg, bg, LINE_OXOX);

 if (outlined) { // Alter the outline so it attaches to our line
  putch(0, y, fg, bg, LINE_XXXO);
  putch(xdim - 1, y, fg, bg, LINE_XOXX);
 }
}

void Window::clear()
{
 w->clear();
}

void Window::refresh()
{
 printf("refreshing window: %dx%d at (%d,%d)\n",xdim,ydim,m_posx,m_posy);
 TCODConsole::blit(w,0,0,0,0,TCODConsole::root,m_posx,m_posy);
 TCODConsole::flush();
}

void init_display()
{
// initscr();
// noecho();
// cbreak();
// keypad(stdscr, true);
// init_colors();
// curs_set(0);
// timeout(1);
// getch();
// timeout(-1);
 TCODConsole::initRoot(100,50,"body and soul", false);
 TCODSystem::setFps(25); // limit framerate to 25 frames per second
 TCODConsole::root->setDefaultBackground(TCODColor::black);
 TCODConsole::root->setDefaultForeground(TCODColor::lightGrey);
}

long input()
{
 TCODConsole::flush();
 TCOD_key_t key;
 TCOD_event_t ev = TCODSystem::waitForEvent(TCOD_EVENT_KEY_RELEASE,&key,0,true);
 switch (key.vk)
 {
  case TCODK_BACKSPACE: return KEY_BACKSPACE;
  case TCODK_ENTER: return '\n';
  case TCODK_UP: return KEY_UP;
  case TCODK_LEFT: return KEY_LEFT;
  case TCODK_RIGHT: return KEY_RIGHT;
  case TCODK_DOWN: return KEY_DOWN;
  case TCODK_ESCAPE: return KEY_ESC;
  default: break;
 }
 return key.c;
}

void debugmsg(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[2048];
 vsprintf(buff, mes, ap);
 va_end(ap);
 //attron(COLOR_PAIR(3));
 //mvprintw(0, 0, "DEBUG: %s      \n  Press spacebar...", buff);
 TCODConsole::root->print(0,0, "DEBUG: %s      \n  Press spacebar...", buff);
 while(input() != ' ');
 //attroff(COLOR_PAIR(3));
}

void refresh_all(bool erase) // erase defaults to false
{
 if (erase)
  TCODConsole::root->clear();

 for (std::list<Window*>::iterator it = WINDOWLIST.begin();
      it != WINDOWLIST.end(); it++)
  (*it)->refresh();
}

std::string key_name(long ch)
{
 switch (ch) {

  case KEY_UP:        return "UP";
  case KEY_RIGHT:     return "RIGHT";
  case KEY_LEFT:      return "LEFT";
  case KEY_DOWN:      return "DOWN";
  case '\n':          return "ENTER";
  case '\t':          return "TAB";
  case KEY_ESC:       return "ESC";
  case KEY_BACKSPACE:
  case 127:
  case 8:             return "BACKSPACE";
  default:
   if (ch < 256) {
    std::stringstream ret;
    ret << char(ch);
    return ret.str();
   } else
    return "???";
 }
 return "???";
}

/*
std::string file_selector(std::string start)
{
 #if (defined _WIN32 || defined __WIN32__)
  debugmsg("Sorry, file_selector() not yet coded for Windows.");
  return;
 #endif
 int winx, winy;
 getmaxyx(stdscr, winx, winy); // Get window size

 Window w_select(0, 0, winx, winy);
 w_select.outline();
 
*/

std::string string_input_popup(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 return string_edit_popup("", buff);
}

std::string string_edit_popup(std::string orig, const char *mes, ...)
{
 std::string ret = orig;
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 int startx = strlen(buff) + 2;
 Window w(0, 11, 80, 3);
 w.outline();
 w.putstr(1, 1, c_ltred, c_black, buff);
 w.putstr(startx, 1, c_magenta, c_black, ret);
 for (int i = startx + ret.length() + 1; i < 79; i++)
  w.putch(i, 1, c_ltgray, c_black, '_');
 int posx = startx + ret.length();
 w.putch(posx, 1, c_ltgray, c_blue, '_');
 do {
  w.refresh();
  long ch = input();
  if (ch == 27) {	// Escape
   return orig;
  } else if (ch == '\n') {
   return ret;
  } else if (ch == KEY_BACKSPACE || ch == 127) {
   if (posx > startx) {
// Move the cursor back and re-draw it
    ret = ret.substr(0, ret.size() - 1);
    w.putch(posx, 1, c_ltgray, c_black, '_');
    posx--;
    w.putch(posx, 1, c_ltgray, c_blue, '_');
   }
  } else {
   ret += ch;
   w.putch(posx, 1, c_magenta, c_black, ch);
   posx++;
   w.putch(posx, 1, c_ltgray, c_blue, '_');
  }
 } while (true);
}

int int_input_popup(const char *mes, ...)
{
 std::string ret;
 bool negative = false;
 va_list ap;
 va_start(ap, mes);
 char buff[1024];
 vsprintf(buff, mes, ap);
 va_end(ap);
 int startx = strlen(buff) + 3;
 Window w(0, 11, 80, 3);
 w.outline();
 w.putstr(1, 1, c_ltred, c_black, buff);
 w.putstr(startx, 1, c_magenta, c_black, ret);
 for (int i = startx + ret.length() + 1; i < 79; i++)
  w.putch(i, 1, c_ltgray, c_black, '_');
 int posx = startx + ret.length();
 w.putch(posx, 1, c_ltgray, c_blue, '_');
 bool done = false;
 while (!done) {
  w.refresh();
  long ch = input();
  if (ch == 27) {	// Escape
   return 0;
  } else if (ch == '\n') {
   done = true;
  } else if (ch == KEY_BACKSPACE || ch == 127) {
   if (posx > startx) {
// Move the cursor back and re-draw it
    ret = ret.substr(0, ret.size() - 1);
    w.putch(posx, 1, c_ltgray, c_black, '_');
    posx--;
    w.putch(posx, 1, c_ltgray, c_blue, '_');
   }
  } else if (ch >= '0' && ch <= '9') {
   ret += ch;
   w.putch(posx, 1, c_magenta, c_black, ch);
   posx++;
   w.putch(posx, 1, c_ltgray, c_blue, '_');
  } else if (ch == '-') {
   negative = !negative;
   if (negative)
    w.putch(startx - 1, 1, c_magenta, c_black, '-');
   else
    w.putch(startx - 1, 1, c_black, c_black, 'x');
  }
 }

 int retnum = 0;
 for (int i = 0; i < ret.length(); i++) {
  int val = ret[i] - '0';
  for (int n = 0; n < (ret.length() - 1 - i); n++)
   val *= 10;
  retnum += val;
 }
 if (negative)
  retnum *= -1;
 return retnum;
}

long popup_getkey(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[8192];
 vsprintf(buff, mes, ap);
 va_end(ap);
 std::string tmp = buff;
 int width = 0;
 int height = 2;
 size_t pos = tmp.find('\n');
 while (pos != std::string::npos) {
  height++;
  if (pos > width)
   width = pos;
  tmp = tmp.substr(pos + 1);
  pos = tmp.find('\n');
 }
 if (width == 0 || tmp.length() > width)
  width = tmp.length();
 width += 2;
 if (height > 25)
  height = 25;
 Window w(int((80 - width) / 2), int((25 - height) / 2), width, height + 1);
 w.outline();
 tmp = buff;
 pos = tmp.find('\n');
 int line_num = 0;
 while (pos != std::string::npos) {
  std::string line = tmp.substr(0, pos);
  line_num++;
  w.putstr(1, line_num, c_white, c_black, line);
  tmp = tmp.substr(pos + 1);
  pos = tmp.find('\n');
 }
 line_num++;
 w.putstr(1, line_num, c_white, c_black, std::string(tmp));
 
 w.refresh();
 long ch = input();
 return ch;
}

int menu_vec(const char *mes, std::vector<std::string> options)
{
 if (options.size() == 0) {
  debugmsg("0-length menu (\"%s\")", mes);
  return -1;
 }
 std::string title = mes;
 int height = 3 + options.size(), width = title.length() + 2;
 for (int i = 0; i < options.size(); i++) {
  if (options[i].length() + 6 > width)
   width = options[i].length() + 6;
 }
 Window w(10, 6, width, height);
 w.outline();
 w.putstr(1, 1, c_white, c_black, title);
 
 for (int i = 0; i < options.size(); i++)
  w.putstr(1, i + 2, c_white, c_black, "%d: %s", i + 1, options[i].c_str());
 long ch;
 w.refresh();
 do
  ch = input();
 while (ch < '1' || ch >= '1' + options.size());
 return (ch - '1');
}

int menu(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char* tmp;
 std::vector<std::string> options;
 bool done = false;
 while (!done) {
  tmp = va_arg(ap, char*);
  if (tmp != NULL) {
   std::string strtmp = tmp;
   options.push_back(strtmp);
  } else
   done = true;
 }
 return (menu_vec(mes, options));
}

void popup(const char *mes, ...)
{
 va_list ap;
 va_start(ap, mes);
 char buff[8192];
 vsprintf(buff, mes, ap);
 va_end(ap);
 std::string tmp = buff;
 int width = 0;
 int height = 2;
 size_t pos = tmp.find('\n');
 while (pos != std::string::npos) {
  height++;
  if (pos > width)
   width = pos;
  tmp = tmp.substr(pos + 1);
  pos = tmp.find('\n');
 }
 if (width == 0 || tmp.length() > width)
  width = tmp.length();
 width += 2;
 if (height > 25)
  height = 25;
 Window w(int((80 - width) / 2), int((25 - height) / 2), width, height + 1);
 w.outline();
 tmp = buff;
 pos = tmp.find('\n');
 int line_num = 0;
 while (pos != std::string::npos) {
  std::string line = tmp.substr(0, pos);
  line_num++;
  w.putstr(1, line_num, c_white, c_black, line.c_str());
  tmp = tmp.substr(pos + 1);
  pos = tmp.find('\n');
 }
 line_num++;
 w.putstr(1, line_num, c_white, c_black, tmp.c_str());
 
 w.refresh();
 char ch;
 do
  ch = input();
 while(ch != ' ' && ch != '\n' && ch != KEY_ESC);
}

bool parse_color_tags(std::string text, std::vector<std::string> &segments,
                      std::vector<long> &color_pairs, nc_color fg, nc_color bg)
{
 size_t tag;
 nc_color cur_fg = fg, cur_bg = bg;

 while ( (tag = text.find("<c=")) != std::string::npos ) {
// Everything before the tag is a segment, with the current colors
  segments.push_back( text.substr(0, tag) );
  color_pairs.push_back( get_color_pair(cur_fg, cur_bg) );
// Strip off everything up to and including "<c="
  text = text.substr(tag + 3);
// Find the end of the tag
  size_t tagend = text.find(">");
  if (tagend == std::string::npos) {
   debugmsg("Unterminated color tag! %d:%s:",
            int(tag), text.c_str());
   return false;
  }
  std::string tag = text.substr(0, tagend);
// Strip out the tag
  text = text.substr(tagend + 1);

  if (tag == "reset" || tag == "/") { // Reset the colors
   cur_fg = fg;
   cur_bg = bg;
  } else { // We're looking for the color!
   size_t comma = tag.find(",");
   if (comma == std::string::npos) { // No comma - just setting fg
    cur_fg = color_string(tag);
    if (cur_fg == c_null) {
     debugmsg("Malformed color tag: %s", tag.c_str());
     return false;
    }
   } else {
    nc_color new_fg = color_string( tag.substr(0, comma) ),
             new_bg = color_string( tag.substr(comma + 1) );
    if (new_fg == c_null && new_bg == c_null) {
     debugmsg("Malformed color tag: %s", tag.c_str());
     return false;
    }
    if (new_fg != c_null)
     cur_fg = new_fg;
    if (new_bg != c_null)
     cur_bg = new_bg;
   } // if comma was found
  } // color needed to be found
 } // while (tag != std::string::npos)
// There's a little string left over; push it into our vectors!
 segments.push_back(text);
 color_pairs.push_back( get_color_pair(cur_fg, cur_bg) );
 printf("pushed back: [%08x]\n", get_color_pair(cur_fg, cur_bg));

 if (segments.size() != color_pairs.size()) {
  debugmsg("Segments.size() = %d, color_pairs.size() = %d",
           segments.size(), color_pairs.size());
  return false;
 }

 return true;
}
