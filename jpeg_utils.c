#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#include "jpeg_utils.h"
void
load_jpeg(const char *filename, ImageViewer *image_viewer)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *file;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Could not open file: %s\n", filename);
		exit(1);
	}

	jpeg_stdio_src(&cinfo, file);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	image_viewer->width = cinfo.output_width;
	image_viewer->height = cinfo.output_height;
	image_viewer->color_components = cinfo.output_components;

	image_viewer->image_buffer =
		(unsigned char *)malloc(image_viewer->width *
			image_viewer->height * image_viewer->color_components);

	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = image_viewer->image_buffer +
			(cinfo.output_scanline) * image_viewer->width *
				image_viewer->color_components;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(file);
}

void
convert_jpeg_to_ximage(ImageViewer *image_viewer)
{
	int depth = DefaultDepth(image_viewer->display, image_viewer->screen);
	int bitmap_pad = (depth == 24) ? 32 : depth;
	int bytes_per_line =
		((image_viewer->width * image_viewer->color_components *
				 bitmap_pad +
			 7) /
			8);

	image_viewer->ximage = XCreateImage(image_viewer->display,
		DefaultVisual(image_viewer->display, image_viewer->screen),
		depth, ZPixmap, 0, (char *)image_viewer->image_buffer,
		image_viewer->width, image_viewer->height, bitmap_pad,
		bytes_per_line);
}

void
free_jpeg_buffer(ImageViewer *image_viewer)
{
	free(image_viewer->image_buffer);
	image_viewer->image_buffer = NULL;
}
