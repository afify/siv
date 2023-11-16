/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <jpeglib.h>
#include <png.h>

#include "image_viewer.h"

void
x_connect(ImageViewer *image_viewer)
{
	image_viewer->display = XOpenDisplay(NULL);
	if (!image_viewer->display) {
		fprintf(stderr, "Could not open display.\n");
		exit(1);
	}
	image_viewer->screen = DefaultScreen(image_viewer->display);
	image_viewer->root =
		RootWindow(image_viewer->display, image_viewer->screen);
	image_viewer->black_pixel =
		BlackPixel(image_viewer->display, image_viewer->screen);
	image_viewer->white_pixel =
		WhitePixel(image_viewer->display, image_viewer->screen);
}

void
create_window(
	ImageViewer *image_viewer, int x_pos, int y_pos, int width, int height)
{
	image_viewer->window =
		XCreateSimpleWindow(image_viewer->display, image_viewer->root,
			x_pos, y_pos, width, height, 0, /* border width */
			image_viewer->black_pixel, /* border pixel */
			image_viewer->white_pixel /* background */
		);

	Atom wm_type = XInternAtom(
		image_viewer->display, "_NET_WM_WINDOW_TYPE", False);
	Atom wm_type_dialog = XInternAtom(
		image_viewer->display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	XChangeProperty(image_viewer->display, image_viewer->window, wm_type,
		XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_type_dialog,
		1);

	XSelectInput(image_viewer->display, image_viewer->window, ExposureMask);
	XMapWindow(image_viewer->display, image_viewer->window);
}

void
draw_image(ImageViewer *image_viewer)
{
	XPutImage(image_viewer->display, image_viewer->window, image_viewer->gc,
		image_viewer->ximage, 0, 0, 0, 0, image_viewer->width,
		image_viewer->height);
	XFlush(image_viewer->display);
}

void
handle_key_press(ImageViewer *image_viewer, XKeyEvent *event)
{
	KeySym key = XLookupKeysym(event, 0);
	if (key == XK_q) {
		XDestroyWindow(image_viewer->display, image_viewer->window);
		XCloseDisplay(image_viewer->display);
		exit(0);
	}
}

void
event_loop(ImageViewer *image_viewer)
{
	XEvent event;
	while (1) {
		XNextEvent(image_viewer->display, &event);
		switch (event.type) {
		case Expose:
			draw_image(image_viewer);
			break;
		case KeyPress:
			handle_key_press(image_viewer, &event.xkey);
			break;
		}
	}
}
