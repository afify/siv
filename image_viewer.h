#ifndef IMAGE_VIEWER_H
#define IMAGE_VIEWER_H

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

typedef struct {
	int width;
	int height;
	int x_pos;
	int y_pos;
	int screen;
	Window root;
	Window window;
	GC gc;
	XImage *ximage;
	Display *display;
	unsigned long black_pixel;
	unsigned long white_pixel;

	// PNG-specific fields
	unsigned char *image_buffer;
	unsigned long image_buffer_size;
	png_byte color_type;
	png_byte bit_depth;
	png_structp png_ptr;
	png_infop info_ptr;

	// JPEG-specific fields
	unsigned int color_components;
	// unsigned char *jpeg_buffer;
	// unsigned long jpeg_buffer_size;
} ImageViewer;

void x_connect(ImageViewer *image_viewer);
void create_window(ImageViewer *image_viewer, int x_pos, int y_pos, int width, int height);
void draw_image(ImageViewer *image_viewer);
void handle_key_press(ImageViewer *image_viewer,XKeyEvent *event);
void event_loop(ImageViewer *image_viewer);
void detect_image_type(const char *filename, ImageViewer *image_viewer);

#endif /* IMAGE_VIEWER_H */
