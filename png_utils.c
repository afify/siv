/* See LICENSE file for copyright and license details. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <png.h>

#include <X11/Xlib.h>
#include <X11/ImUtil.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "image_viewer.h"

void load_png(const char *filename, ImageViewer *image_viewer);
void convert_png_to_ximage(ImageViewer *image_viewer);
// void draw_image(const ImageViewer *image_viewer);
void free_png_data(ImageViewer *image_viewer);

void
load_png(const char *filename, ImageViewer *image_viewer)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		fprintf(stderr, "Error opening PNG file: %s\n", filename);
		exit(1);
	}

	png_structp png_ptr =
		png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fclose(fp);
		fprintf(stderr, "Error creating PNG read struct\n");
		exit(1);
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		fclose(fp);
		fprintf(stderr, "Error creating PNG info struct\n");
		exit(1);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		fprintf(stderr, "Error during PNG read\n");
		exit(1);
	}

	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);

	image_viewer->width = png_get_image_width(png_ptr, info_ptr);
	image_viewer->height = png_get_image_height(png_ptr, info_ptr);
	image_viewer->color_type = png_get_color_type(png_ptr, info_ptr);
	image_viewer->bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (image_viewer->bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_GRAY &&
		image_viewer->bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_RGB ||
		image_viewer->color_type == PNG_COLOR_TYPE_GRAY ||
		image_viewer->color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_GRAY ||
		image_viewer->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	png_read_update_info(png_ptr, info_ptr);

	int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	image_viewer->image_buffer =
		(unsigned char *)malloc(row_bytes * image_viewer->height);
	if (!image_viewer->image_buffer) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		fprintf(stderr,
			"Error allocating memory for PNG image buffer\n");
		exit(1);
	}

	png_bytepp row_pointers =
		(png_bytepp)malloc(sizeof(png_bytep) * image_viewer->height);
	if (!row_pointers) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		free(image_viewer->image_buffer);
		fprintf(stderr,
			"Error allocating memory for PNG row pointers\n");
		exit(1);
	}

	for (int y = 0; y < image_viewer->height; y++) {
		row_pointers[y] = image_viewer->image_buffer + y * row_bytes;
	}

	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, NULL);

	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
}

void
convert_png_to_ximage(ImageViewer *image_viewer)
{
	int depth = DefaultDepth(image_viewer->display, image_viewer->screen);
	XImage *ximage = XCreateImage(image_viewer->display, CopyFromParent,
		depth, ZPixmap, 0, (char *)image_viewer->image_buffer,
		image_viewer->width, image_viewer->height, 32, 0);
	if (!ximage) {
		fprintf(stderr, "Error creating XImage\n");
		exit(1);
	}

	image_viewer->ximage = ximage;
}

// void
// draw_image(const ImageViewer *image_viewer)
// {
// 	XPutImage(image_viewer->display, image_viewer->window, image_viewer->gc,
// 	          image_viewer->ximage, 0, 0, 0, 0, image_viewer->width,
// 	          image_viewer->height);
// }

void
free_png_data(ImageViewer *image_viewer)
{
	free(image_viewer->image_buffer);
	XDestroyImage(image_viewer->ximage);
}
