/*
 * $Id: info.c,v 1.1 1996/07/11 15:37:31 grubba Exp $
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
 * Globals
 */

#include "lyslogo.xpm"

/*
 * Functions
 */

void *info_window_main(void *arg)
{
  Display *display;

  if ((display = XOpenDisplay(NULL))) {
    Screen *screen = DefaultScreenOfDisplay(display);
    int screen_num = DefaultScreen(display);
    Window info_window;
    Colormap default_cmap;
    int backgroundPixel;
    int darkgreyPixel;
    int dummy;
    int width;
    int baseline;
    struct my_x_string *my_strings = NULL;
    GC big_font_gc;
    XFontStruct *big_font;
    GC medium_font_gc;
    XFontStruct *medium_font;
    GC small_font_gc;
    XFontStruct *small_font;

    XCharStruct char_limits;

    XImage *logotype = NULL;
    XImage *logotype_mask = NULL;
    XWindowAttributes window_attributes;
    XpmAttributes xpm_attribs;

    const char *lmtae = "LMT Amiga";
    const char *authors1 = "by Henrik Grubbström";
    const char *authors2 = "Marcus Comstedt and Per Hedbor";
    const char *release = "Beta release 0.01a";
    const char *copyright = "Copyright 1996 The authors and Lysator.";
    const char *distribution = "Distributed uder GPL.";

    int status;

    default_cmap = DefaultColormap(display, screen_num);

    backgroundPixel = gui_AllocColor(display, default_cmap, 168, 168, 168,
				     WhitePixel(display, screen_num));

    darkgreyPixel = gui_AllocColor(display, default_cmap, 80, 80, 80,
				   BlackPixel(display, screen_num));
    
    /* 368x130+47+139 */
    info_window = gui_CreateSimpleWindow(display, RootWindow(display, screen_num),
					 "Info", "Info",
					 47, 139, 368, 130, 0,
					 BlackPixel(display, screen_num),
					 backgroundPixel);

    if (XGetWindowAttributes(display, info_window, &window_attributes) < 0) {
      xpm_attribs.valuemask = XpmCloseness;
      fprintf(stderr, "Couldn't get attributes\n");
    } else {
      xpm_attribs.visual = window_attributes.visual;
      xpm_attribs.depth = window_attributes.depth;
      xpm_attribs.colormap = window_attributes.colormap;
      xpm_attribs.valuemask = XpmVisual | XpmDepth | XpmColormap | XpmCloseness;
    }

    if ((status = XpmCreateImageFromData(display, lyslogo,
					 &logotype, &logotype_mask, NULL)) < 0) {
      fprintf(stderr, "Error %d creating logotype (\"%s\")\n",
	      status, XpmGetErrorString(status));
    }

    big_font_gc = my_create_font_gc(display, screen_num, info_window, 
			      "-*-times-bold-r-*-*-34-*-*-*-*-*-iso8859-1",
				    &big_font);

    width = XTextWidth(big_font, lmtae, strlen(lmtae));

    XTextExtents(big_font, lmtae, strlen(lmtae),
		 &dummy, &dummy, &dummy, &char_limits);

    baseline = 14 + char_limits.ascent;

    my_strings = new_my_x_string(lmtae, 368 - 54 - width, baseline,
				 big_font_gc, my_strings);

    baseline += big_font->descent;
    
    medium_font_gc = my_create_font_gc(display, screen_num, info_window,
				       "-*-helvetica-medium-r-*-*-14-*-*-*-*-*-iso8859-1",
				       &medium_font);

    width = XTextWidth(medium_font, authors1, strlen(authors1));

    baseline += 4 + medium_font->ascent;

    my_strings = new_my_x_string(authors1, 368 - 54 - width, baseline,
				 medium_font_gc, my_strings);

    baseline += medium_font->descent;

    small_font_gc = my_create_font_gc(display, screen_num, info_window,
				      "-*-helvetica-medium-r-*-*-10-*-*-*-*-*-iso8859-1",
				      &small_font);

    XSetForeground(display, small_font_gc, darkgreyPixel);

    width = XTextWidth(small_font, release, strlen(release));

    baseline += 15 + small_font->ascent;

    my_strings = new_my_x_string(release, 368 - 15 - width, baseline,
				 small_font_gc, my_strings);

    baseline += small_font->descent;

    width = XTextWidth(small_font, copyright, strlen(copyright));

    baseline += 1 + small_font->ascent;

    my_strings = new_my_x_string(copyright, 368 - 15 - width, baseline,
				 small_font_gc, my_strings);

    baseline += small_font->descent;

    width = XTextWidth(small_font, distribution, strlen(distribution));

    baseline += 1 + small_font->ascent;

    my_strings = new_my_x_string(distribution, 368 - 15 - width, baseline,
				 small_font_gc, my_strings);

    baseline += small_font->ascent;

    XSelectInput(display, info_window, ExposureMask);

    XMapWindow(display, info_window);

    while (1) {
      XEvent event;

      XNextEvent(display, &event);

      switch (event.type) {
      case Expose:

	if (event.xexpose.count) {
	  break;
	}

	if (logotype) {
	  XPutImage(display, info_window, big_font_gc, logotype, 0, 0,
		    5, 16, logotype->width, logotype->height);
	}

	{
	  struct my_x_string *str = my_strings;

	  while (str) {
	    XDrawString(display, info_window, str->gc,
			str->x, str->y, str->string, str->str_len);
	    str = str->next;
	  }
	}
	XFlush(display);
    
	break;
      }
    }
    
  } else {
    fprintf(stderr, "Couldn't connect to display \"%s\"\n", XDisplayName(NULL));
    exit(1);
  }
}

int start_info_window(void)
{
  thread_t info_thread;

  return(thr_create(NULL, 0, info_window_main, NULL,
		    THR_DETACHED | THR_DAEMON, &info_thread));
}
