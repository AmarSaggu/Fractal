#include "Image.h"

#include <sys/mman.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include <stdio.h>
#include <stdlib.h>

struct Image *image_create(size_t width, size_t height, const char *filename)
{
	struct Image *image = malloc(sizeof(*image));
	
	if (!image) {
		return NULL;
	}
	
	// Open file
	int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
	
	if (fd == -1) {
		perror("Error opening file");
		free(image);	
		return NULL;
	}
	
	// Create PPM image header
	char *header;
	int header_length = asprintf(&header, "P6\n%d %d\n255\n", width, height);

	if (header_length == -1) {
		perror("Failed to allocate header");
		close(fd);
		free(image);
		return NULL;
	}
	
	// Write header to file
	write(fd, header, header_length);
	free(header);
	header = NULL;
	
	// Expand file to the full image length
	size_t array_size = width * height * 3; // 3 bytes per pixel
	size_t file_size = header_length + array_size;
	
	lseek(fd, array_size - 1, SEEK_CUR);
	write(fd, "", 1);
	
	// Map file to memory
	char *map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (map == MAP_FAILED) {
		perror("Error mapping file");
		close(fd);
		free(image);
		return NULL;
	}
	
	image->width = width;
	image->height = height;
	image->fd = fd;
	image->array_size = array_size;
	image->file_size = file_size;
	image->map = map;
	image->array = map + header_length;

	return image;
}

void image_close(struct Image *image)
{
	if(munmap(image->map, image->file_size) == -1) {
		perror("Failed to unmap file");
	}
	
	close(image->fd);
	free(image);
}
