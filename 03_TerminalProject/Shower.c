#define NCURSES_WIDECHAR 1
#include <curses.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

#define F 3

int main(int argc, char *argv[]) {
  WINDOW *win, *bwin;
  wchar_t *buf;
  wchar_t **lines;
  wchar_t c;
  struct stat info;
  int j, i, nlines, done, X, Y, cx, cy, size;
  FILE *fp;

  setlocale(LC_ALL, "");
  if(argc<2)
    return fprintf(stderr, "Usage: %s <filename>\n", basename(argv[0])), 1;

  stat(argv[1], &info);
  if(!(buf = calloc(info.st_size+1, sizeof(wchar_t))))
    error(2, errno, "Allocating buffer");
  fp = fopen(argv[1], "rt,ccs=UTF-8");
  for(size=nlines=0; (c=fgetwc(fp)) != WEOF; buf[size++] = c) {
    if(c == L'\n') {
      nlines++;
      c = 0;
    }
  }
  fclose(fp);
  buf[size] = L'0';

  lines = calloc(nlines+1, sizeof(wchar_t *));
  for(lines[i=j=0]=buf; i<size; i++)
    if(!buf[i])
      lines[++j] = buf + i + 1;
  
  initscr();
  start_color();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_CYAN, COLOR_BLACK);
  noecho();
  cbreak();

  bwin = newwin(LINES-2*F+2, COLS-2*F+2, F-1, F-1);
  win = newwin(LINES-2*F, COLS-2*F, F, F);

  attron(COLOR_PAIR(2));
  wattron(bwin, COLOR_PAIR(1));

  mvprintw(0, 0, "Файл %s: %d символов", argv[1], size);
  refresh();

  keypad(win, TRUE);

  done = FALSE;
  X = Y = cx = cy = c = 0;
  while(!done) {
    box(bwin, 0, 0);
    if(c)
      mvwaddstr(bwin, 0, 2, keyname(c));
    wrefresh(bwin);
    werase(win);
    for(i=0; i <= win->_maxy && i+Y < nlines; i++) {
      wattron(win, COLOR_PAIR(3));
      mvwprintw(win, i, 0, "%4d", i+Y+1);
      wattroff(win, COLOR_PAIR(3));
      mvwaddnwstr(win, i, 5, wcslen(lines[i+Y])>X ? lines[i+Y]+X : L"", win->_maxx-4);
    }
    wmove(win, cy, 5+cx);
    switch(c = wgetch(win)) {
      case 'q':
      case 27: done = TRUE; break;
      case KEY_NPAGE: Y = Y+win->_maxy < nlines ? Y+win->_maxy : Y; break;
      case KEY_PPAGE: Y = Y-win->_maxy >= 0 ? Y-win->_maxy : 0; break;
      case KEY_DOWN: Y = Y+1 < nlines ? Y+1 : Y; break;
      case KEY_UP: Y = Y > 0 ? Y-1 : Y; break;
      case KEY_RIGHT: X++; break;
      case KEY_LEFT: X = X>0 ? X-1 : X; break;
      case KEY_HOME: Y = 0; break;
      case KEY_END: Y = nlines-win->_maxy-1; break;
    }
    
  }

  free(buf);
  free(lines);
  endwin();
  return 0;
}
