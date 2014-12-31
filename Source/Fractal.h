#pragma once

#include "Image.h"

#include <pthread.h>

#include <complex.h>
#include <stdint.h>
#include <string.h>

struct Fractal
{
	struct Image *image;
	
	unsigned int width, height;
	
	long double complex position;
	long double zoom;
	
	int line;
	
	pthread_mutex_t lock;
};

struct Fractal *Fractal_Create(char *, int, int, long double complex, long double);
void Fractal_Destroy(struct Fractal *);

void *Fractal_Render(void *);
