#pragma once

#include <strings.h>
#include <stdint.h>

/*
A small image class to create and manipulate image files.

It uses the uncompressed PPM image format and maps the image
data to memory, allowing direct manipulation of the image
file!

Huge (HUGE) images can be created, since the image is never present
in RAM. Files in the order of ~50GB or more! Note that you may need
to temporarily increase the number of memory maps the program can
have if working with large files, by running as root:

sysctl vm.max_map_count=n
(where n > 65536, which is the default limit)
*/

struct Image {
	size_t width, height;
	
	int fd;
	size_t array_size, file_size;
	
	char *map;
	uint8_t *array;
};

struct Image *image_create(size_t width, size_t height, const char *filename);
void image_close(struct Image *image);
