/*
 * $Id: gui.h,v 1.3 1996/07/17 16:14:24 grubba Exp $
 *
 * User Interface for the M68000 to Sparc recompiler
 *
 * $Log: gui.h,v $
 * Revision 1.2  1996/07/11 20:14:21  grubba
 * Added gui_AddButton().
 *
 * Revision 1.1  1996/07/11 15:38:57  grubba
 * Graphics User-Interface files.
 * Initial version.
 *
 *
 */

#ifndef GUI_H
#define GUI_H

/*
 * Structures
 */

struct my_x_string {
  struct my_x_string *next;

  int x, y;

  GC gc;

  const char *string;

  int str_len;
};

/*
 * Prototypes
 */

Window gui_CreateSimpleWindow(Display *display, Window root,
			      const char *window_name, const char *icon_name,
			      int x, int y, unsigned width, unsigned height,
			      unsigned border_width,
			      unsigned long border_color, unsigned long background);
int gui_AllocColor(Display *display, Colormap cmap,
		   int red, int green, int blue, int fallback);
void gui_AddButton(const char *label, void (*callback)(void *), void *callbackarg,
		   int x, int y, unsigned width, unsigned height,
		   Window window, GC gc, XFontStruct *font,
		   int backgroundPixel, int shadowPixel);

struct my_x_string *new_my_x_string(const char *string, int x, int y,
				    GC gc, struct my_x_string *last_str);
GC my_create_font_gc(Display *display, int screen_num, Window window,
		     const char *font_name, XFontStruct **font_info);
void *info_window_main(void *arg);
int start_info_window(void);
void *disassembler_main(void *arg);
int start_disassembler(void);
void *register_monitor_main(void *arg);
void start_register_monitor(const char *name, volatile U32 *addr,
			    int x, int y, unsigned width, unsigned height,
			    Window root, GC labelgc, GC boxgc,
			    int bgPixel, int dgPixel);
void *register_dump_main(void *arg);
int start_register_dump(struct m_registers *regs);

#endif /* GUI_H */
