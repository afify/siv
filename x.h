/* See LICENSE file for copyright and  license details. */

#ifndef X_H
#define X_H

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>

#include "image_viewer.h"

void x_connect(ImageViewer *image_viewer);
void create_window(int x_pos, int y_pos, int width, int height);
void draw_image(ImageViewer *image_viewer);
void handle_key_press(XKeyEvent *event);
void event_loop(ImageViewer *image_viewer);
void detect_image_type(const char *filename, ImageViewer *image_viewer);

#endif /* X_H */
