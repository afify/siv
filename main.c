#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image_viewer.h"
#include "jpeg_utils.h"

void
detect_file_type(const char *filename)
{
	const char *extension = strrchr(filename, '.');
	if (extension && strcmp(extension, ".jpg") == 0) {
		printf("File type: JPEG\n");
	} else if (extension && strcmp(extension, ".png") == 0) {
		printf("File type: PNG\n");
	} else {
		printf("Unsupported file type.\n");
	}
}

int
main(int argc, char **argv)
{
	if (argc < 4) {
		fprintf(stderr, "Usage: %s <image.png> <x_pos> <y_pos>\n",
		        argv[0]);
		return 1;
	}

	ImageViewer image_viewer;

	const char *image_filename = argv[1];
	image_viewer.x_pos = atoi(argv[2]);
	image_viewer.y_pos = atoi(argv[3]);

	detect_file_type(image_filename);

	//load_jpeg(filename, &image_viewer);
	//convert_jpeg_to_ximage(&image_viewer);

	/* Perform image viewer initialization */
	image_viewer.display = XOpenDisplay(NULL);
	if (!image_viewer.display) {
		fprintf(stderr, "Unable to open the display.\n");
		exit(1);
	}

	image_viewer.screen = DefaultScreen(image_viewer.display);
	image_viewer.root =
		RootWindow(image_viewer.display, image_viewer.screen);

	image_viewer.window =
		XCreateSimpleWindow(image_viewer.display, image_viewer.root, 0,
	                            0, image_viewer.width, image_viewer.height,
	                            0, 0, WhitePixel(image_viewer.display, 0));
	XMapWindow(image_viewer.display, image_viewer.window);

	image_viewer.gc =
		XCreateGC(image_viewer.display, image_viewer.window, 0, 0);

	XSetForeground(image_viewer.display, image_viewer.gc,
	               BlackPixel(image_viewer.display, 0));
	XSetBackground(image_viewer.display, image_viewer.gc,
	               WhitePixel(image_viewer.display, 0));

	XSelectInput(image_viewer.display, image_viewer.window,
	             ExposureMask | KeyPressMask);
	XEvent event;

	while (1) {
		XNextEvent(image_viewer.display, &event);

		if (event.type == Expose) {
			draw_image(&image_viewer);
		}

		if (event.type == KeyPress) {
			break;
		}
	}

	/* Clean up resources */
	//free_jpeg_buffer(&image_viewer);
	XFreeGC(image_viewer.display, image_viewer.gc);
	XDestroyWindow(image_viewer.display, image_viewer.window);
	XCloseDisplay(image_viewer.display);

	return 0;
}
