#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "image_viewer.h"
#include "utils.h"
#include "x.h"

#define XA_ATOM 4

/* Load the PNG image */
static void
load_png(ImageViewer *image_viewer, const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open file: %s\n", filename);
		exit(1);
	}

	unsigned char header[8];
	fread(header, 1, 8, file);
	if (png_sig_cmp(header, 0, 8)) {
		fprintf(stderr, "File is not a valid PNG image: %s\n",
			filename);
		exit(1);
	}

	image_viewer->png_ptr =
		png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!image_viewer->png_ptr) {
		fprintf(stderr, "Could not create PNG read struct.\n");
		exit(1);
	}

	image_viewer->info_ptr = png_create_info_struct(image_viewer->png_ptr);
	if (!image_viewer->info_ptr) {
		fprintf(stderr, "Could not create PNG info struct.\n");
		exit(1);
	}

	if (setjmp(png_jmpbuf(image_viewer->png_ptr))) {
		fprintf(stderr, "Error during PNG file reading.\n");
		exit(1);
	}

	png_init_io(image_viewer->png_ptr, file);
	png_set_sig_bytes(image_viewer->png_ptr, 8);

	png_read_info(image_viewer->png_ptr, image_viewer->info_ptr);

	image_viewer->width = png_get_image_width(
		image_viewer->png_ptr, image_viewer->info_ptr);
	image_viewer->height = png_get_image_height(
		image_viewer->png_ptr, image_viewer->info_ptr);
	image_viewer->color_type = png_get_color_type(
		image_viewer->png_ptr, image_viewer->info_ptr);
	image_viewer->bit_depth = png_get_bit_depth(
		image_viewer->png_ptr, image_viewer->info_ptr);

	if (image_viewer->bit_depth == 16) {
		png_set_strip_16(image_viewer->png_ptr);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(image_viewer->png_ptr);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_GRAY &&
		image_viewer->bit_depth < 8) {
		png_set_expand_gray_1_2_4_to_8(image_viewer->png_ptr);
	}

	if (png_get_valid(image_viewer->png_ptr, image_viewer->info_ptr,
		    PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(image_viewer->png_ptr);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_RGB ||
		image_viewer->color_type == PNG_COLOR_TYPE_GRAY ||
		image_viewer->color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_filler(image_viewer->png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

	if (image_viewer->color_type == PNG_COLOR_TYPE_GRAY ||
		image_viewer->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(image_viewer->png_ptr);
	}

	png_read_update_info(image_viewer->png_ptr, image_viewer->info_ptr);

	image_viewer->ximage = XCreateImage(image_viewer->display,
		DefaultVisual(image_viewer->display, image_viewer->screen),
		DefaultDepth(image_viewer->display, image_viewer->screen),
		ZPixmap, 0, NULL, image_viewer->width, image_viewer->height, 32,
		0);

	if (!image_viewer->ximage) {
		fprintf(stderr, "Could not create XImage.\n");
		exit(1);
	}

	image_viewer->ximage->data =
		malloc(image_viewer->ximage->bytes_per_line *
			image_viewer->ximage->height);

	png_bytep *row_pointers =
		(png_bytep *)malloc(sizeof(png_bytep) * image_viewer->height);
	for (int y = 0; y < image_viewer->height; y++) {
		row_pointers[y] = image_viewer->ximage->data +
			y * image_viewer->ximage->bytes_per_line;
	}

	png_read_image(image_viewer->png_ptr, row_pointers);
	png_read_end(image_viewer->png_ptr, NULL);

	fclose(file);
	free(row_pointers);
}

int
main(int argc, char **argv)
{
	ImageViewer *image_viewer;
	image_viewer = calloc(1, sizeof(ImageViewer));
	int opt;

	int x_value = -1;
	int y_value = -1;
	char *image_filename = NULL;
	image_viewer->x_pos = 0;
	image_viewer->y_pos = 0;

	while ((opt = getopt(argc, argv, "x:y:f:")) != -1) {
		switch (opt) {
		case 'x':
			x_value = atoi(optarg);
			image_viewer->x_pos = x_value;
			break;
		case 'y':
			y_value = atoi(optarg);
			image_viewer->y_pos = y_value;
			break;
		case 'f':
			image_filename = optarg;
			break;
		case '?':
			fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			return 1;
		case ':':
			fprintf(stderr, "Option -%c requires an argument.\n",
				optopt);
			return 1;
		}
	}

	if (x_value < 0 || y_value < 0 ) {
		fprintf(stderr, "Both -x and -y flags are mandatory.\n");
		return 1;
	}

	if (image_filename == NULL) {
		fprintf(stderr, "Option -f for file name is mandatory.\n");
		return 1;
	}

	x_connect(image_viewer);
	load_png(image_viewer, image_filename);
	//load_jpeg(image_viewer, image_filename);
	create_window(image_viewer, image_viewer->x_pos, image_viewer->y_pos,
		image_viewer->width, image_viewer->height);

	image_viewer->gc =
		XCreateGC(image_viewer->display, image_viewer->window, 0, NULL);
	XSetBackground(image_viewer->display, image_viewer->gc,
		image_viewer->white_pixel);
	XSetForeground(image_viewer->display, image_viewer->gc,
		image_viewer->black_pixel);
	XSelectInput(image_viewer->display, image_viewer->window,
		ExposureMask | KeyPressMask);
	XMapRaised(image_viewer->display, image_viewer->window);
	event_loop(image_viewer);
	return 0;
}
