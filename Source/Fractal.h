#pragma once

#include <pthread.h>

#include <complex.h>
#include <stdint.h>
#include <string.h>

struct Fractal
{
	unsigned int width, height;
	
	int fd;
	uint8_t *map, *array;
	
	size_t file_size, array_size;

	long double complex position;
	long double zoom;
	
	int line;
	
	pthread_mutex_t lock;
};

struct Fractal *Fractal_Create(char *, int, int, long double complex, long double);
void Fractal_Destroy(struct Fractal *);

void *Fractal_Render(void *);
