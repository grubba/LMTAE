/*
 * $Id: regdump.c,v 1.1 1996/07/11 15:37:37 grubba Exp $
 *
 * User Interface for the M68000 to Sparc recompiler
 *
 * $Log$
 *
 */

/*
 * Includes
 */

#include <thread.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include "../recomp.h"
#include "../m68k.h"
#include "../gui.h"

/*
 * Structures
 */

struct regmon_args {
  const char *label;
  volatile ULONG *addr;
  int x, y;
  unsigned width, height;
  Window root;
  GC labelgc;
  GC boxgc;
  int backgroundPixel, shadowPixel;
};

/*
 * Functions
 */

void *register_monitor_main(void *arg)
{
  Display *display;
  struct regmon_args *args = arg;

  if (args && (display = XOpenDisplay(NULL))) {
    int screen_num = DefaultScreen(display);
    Window window;
    GC localgc;
    XGCValues gcvalues;
    int divider = strlen(args->label)*6 + 2;
    fd_set read_fds;
    struct timeval timeout;

    window = XCreateSimpleWindow(display, args->root, args->x, args->y,
				 args->width, args->height,
				 0, BlackPixel(display, screen_num),
				 WhitePixel(display, screen_num));

    localgc = XCreateGC(display, window, 0, &gcvalues);

    XSelectInput(display, window,
		 ExposureMask | EnterWindowMask | FocusChangeMask |
		 ButtonPressMask | ButtonReleaseMask);

    XMapWindow(display, window);

    FD_ZERO(&read_fds);

    while (1) {
      char string[30];
      XEvent event;

      FD_SET(display->fd, &read_fds);
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      if (select(display->fd + 1, &read_fds, NULL, NULL, &timeout) < 0) {
	fprintf(stderr, "selection error\n");
	break;
      }

      while (XCheckMaskEvent(display, ~0, &event)) {

	switch(event.type) {
	case Expose:

	  if (event.xexpose.count) {
	    break;
	  }

	  XSetForeground(display, localgc, WhitePixel(display, screen_num));

	  XFillRectangle(display, window, localgc,
			 divider, 0, args->width - divider, args->height);
	  
	  XSetForeground(display, localgc, args->backgroundPixel);
	  XFillRectangle(display, window, localgc,
			 0, 0, divider, args->height);

	  XDrawLine(display, window, localgc,
		    divider, args->height-2, args->width-2, args->height-2);
	  XDrawLine(display, window, localgc,
		    args->width-2, args->height-2, args->width-2, 0);

	  XSetForeground(display, localgc, args->shadowPixel);
	  XDrawLine(display, window, localgc,
		  divider, args->height-2, divider, 0);
	  XDrawLine(display, window, localgc,
		    divider+1, args->height-3, divider+1, 0);
	  XDrawLine(display, window, localgc,
		    divider+1, 0, args->width-1, 0);
	  XDrawLine(display, window, localgc,
		    divider+1, 1, args->width-2, 1);

	  XSetForeground(display, localgc, BlackPixel(display, screen_num));

	  XDrawImageString(display, window, args->labelgc,
			   0, 12, args->label, strlen(args->label));

	  break;
	default:
	  break;
	}
      }
      sprintf(string, "%08lx", *(args->addr));
	  
      XDrawImageString(display, window, args->boxgc,
		       divider + 5, 12, string, strlen(string));

      XFlush(display);
    }
  } else {
    fprintf(stderr, "register_monitor_main failed\n");
  }
  return(NULL);
}

void start_register_monitor(const char *name, volatile ULONG *addr,
			    int x, int y, unsigned width, unsigned height,
			    Window root, GC labelgc, GC boxgc,
			    int bgPixel, int dgPixel)
{
  thread_t monitor_thread;
  struct regmon_args *regmon_args;

  if ((regmon_args = (struct regmon_args *)malloc(sizeof(struct regmon_args)))) {
    regmon_args->label = name;
    regmon_args->addr = addr;
    regmon_args->x = x;
    regmon_args->y = y;
    regmon_args->width = width;
    regmon_args->height = height;
    regmon_args->root = root;
    regmon_args->labelgc = labelgc;
    regmon_args->boxgc = boxgc;
    regmon_args->backgroundPixel = bgPixel;
    regmon_args->shadowPixel = dgPixel;

    thr_create(NULL, 0, register_monitor_main, (void *)regmon_args,
	       THR_DETACHED | THR_DAEMON, &monitor_thread);
  }
}

void *register_dump_main(void *arg)
{
  struct m_registers *regs = (struct m_registers *)arg;
  Display *display;

  if (regs && (display = XOpenDisplay(NULL))) {
    int i;
    char buffer[40];
    int screen_num = DefaultScreen(display);
    Window window;
    Colormap cmap = DefaultColormap(display, screen_num);
    int bgPixel = gui_AllocColor(display, cmap, 168, 168, 168,
				 WhitePixel(display, screen_num));
    int dgPixel = gui_AllocColor(display, cmap, 80, 80, 80,
				 BlackPixel(display, screen_num));
    XGCValues gcvalues;
    GC labelgc;
    GC boxgc;
    XFontStruct *font_info;

  /* 235x348+483+614 */

    window = gui_CreateSimpleWindow(display, RootWindow(display, screen_num),
				    "CPU Monitor", "CPU Monitor",
				    483, 614, 235, 348, 0,
				    BlackPixel(display, screen_num),
				    bgPixel);

    
    gcvalues.foreground = BlackPixel(display, screen_num);
    gcvalues.background = bgPixel;
    gcvalues.line_width = 1;
    gcvalues.line_style = LineSolid;
    gcvalues.fill_style = FillSolid;
    labelgc = XCreateGC(display, window,
			GCForeground | GCBackground | GCLineWidth | GCFillStyle,
			&gcvalues);

    gcvalues.background = WhitePixel(display, screen_num);

    boxgc = XCreateGC(display, window,
		      GCForeground | GCBackground | GCLineWidth | GCFillStyle,
		      &gcvalues);

    if (!(font_info = XLoadQueryFont(display, "-*-clean-medium-r-*-*-12-*-*-*-*-*-iso8859-1"))) {
      font_info = XLoadQueryFont(display, "-*-*-*-r-*-*-12-*-*-*-*-*-iso8859-1");
    }

    if (font_info) {
      XSetFont(display, labelgc, font_info->fid);
      XSetFont(display, boxgc, font_info->fid);
    }

    XFlush(display);	/* Probably needed to avoid race */

    for (i=0; i<8; i++) {
      sprintf(buffer, "D%d:", i);

      start_register_monitor(strdup(buffer), ((ULONG *)regs)+i, 31, 13+24*i, 81, 21,
			     window, labelgc, boxgc, bgPixel, dgPixel);
    }

    for (i=0; i<8; i++) {
      sprintf(buffer, "A%d:", i);

      start_register_monitor(strdup(buffer), ((ULONG *)regs)+8+i, 119, 13+24*i, 81, 21,
			     window, labelgc, boxgc, bgPixel, dgPixel);
    }

    start_register_monitor("Supervisor Stack:", &regs->ssp, 27, 212, 173, 21,
			   window, labelgc, boxgc, bgPixel, dgPixel);

    start_register_monitor("Program Counter:", &regs->pc, 33, 236, 167, 21,
			   window, labelgc, boxgc, bgPixel, dgPixel);

    start_register_monitor("Status Register:", &regs->sr, 33, 260, 167, 21,
			   window, labelgc, boxgc, bgPixel, dgPixel);

    XMapWindow(display, window);

    XFlush(display);
  }
  return(NULL);
}

int start_register_dump(struct m_registers *regs)
{
  thread_t register_thread;
  
  return(thr_create(NULL, 0, register_dump_main, (void *)regs,
		    THR_DETACHED | THR_DAEMON, &register_thread));
}

