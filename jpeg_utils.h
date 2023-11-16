/* See LICENSE file for copyright and  license details. */

#ifndef JPEG_UTILS_H
#define JPEG_UTILS_H

#include "image_viewer.h"

void load_jpeg(const char *filename, ImageViewer *image_viewer);
void convert_jpeg_to_ximage(ImageViewer *image_viewer);
void free_jpeg_buffer(ImageViewer *image_viewer);

#endif /* JPEG_UTILS_H */
