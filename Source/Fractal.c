#include "Fractal.h"
#include "Image.h"

#include <pthread.h>

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include <complex.h>
#include <math.h>

#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)

#define ANTIALIASING 2
#define MAX_ITER 25000

static long double escape;
static long double escape_squared;

static long double log_escape;
static long double log_two;

// Count the digits in a positive number
static unsigned int GetDigitCount(unsigned int num)
{
	unsigned int digit_count = 1;

	while( num != 0 ) {
		num /= 10;
		digit_count++;
	}
		
	return digit_count;
}

struct Fractal *Fractal_Create(char *filename, int width, int height, long double complex position, long double zoom)
{
	struct Fractal *fractal = malloc(sizeof (struct Fractal));
	
	if (!fractal) {
		return NULL;
	}
	
	struct Image *image = image_create(width, height, filename);

	if (!image) {
		free(fractal);
		return NULL;
	}
	
	pthread_mutex_init(&fractal->lock, NULL);
	
	fractal->image = image;
	fractal->width = width;
	fractal->height = height;
	fractal->position = position;
	fractal->zoom = zoom;	
	fractal->line = 0;
	
	// Initialise static variables
	escape = 1000.0l;
	escape_squared = escape * escape;

	log_escape = logl(escape);
	log_two = logl(2.0l);
	
	return fractal;
}

void Fractal_Destroy(struct Fractal *fractal)
{
	image_close(fractal->image);
	
	pthread_mutex_destroy(&fractal->lock);
	
	free(fractal);
}

static inline uint8_t Wrap(unsigned int num)
{
	num %= 510;
	
	if (num > 255) {
		return 254 - num;
	}	
	return num;
}

static int aquire_line(struct Fractal *fractal)
{
	// Lock to aquire new line
	pthread_mutex_lock(&fractal->lock);
	
	int line = fractal->line;
	
	if (line < fractal->height) {
		// Point to next horizontal line
		fractal->line++;
		
		// Print progress
		printf("\r%d / %d", fractal->line, fractal->height);
		fflush(stdout);
	}
	pthread_mutex_unlock(&fractal->lock);
	return line;
}

static unsigned int iterate(long double complex num)
{
	long double complex c = num;
					
	for (unsigned int i = 1; i < MAX_ITER; ++i) {
		num *= num;
		num += c;
		long double length = creall(num) * creall(num) + cimagl(num) * cimagl(num);
		
		if (unlikely(length >= escape_squared)) {
			long double moo = i + 1.0l - (logl(logl(cabs(num)) / log_escape) / log_two);
			return logl(moo) * 175.0l;
		}
	}
	return 0.0l;
}

void *Fractal_Render(void *arg)
{
	struct Fractal *fractal = arg;
	uint8_t *array = fractal->image->array;
	
	long double ratio = fractal->height;
	ratio /= fractal->width;
	
	for (;;) {		
		int line = aquire_line(fractal);
		
		if (line >= fractal->height) {
			return NULL;
		}
	
		for (unsigned int x = 0; x < fractal->width; ++x) {
			long double num_real = ((x + 0.5l) / fractal->width - 0.5l) * 2.0l;
			long double num_imag = ((line + 0.5l) / fractal->height - 0.5l) * 2.0l * ratio;
			long double complex num = num_real + num_imag * I;
			num *= fractal->zoom;
			num += fractal->position;
			
			long double acc = 0.0l;
			
			for (unsigned int ay = 0; ay < ANTIALIASING; ++ay) {
				for(unsigned int ax = 0; ax < ANTIALIASING; ++ax) {
					long double offset_real = (((ax + 0.5l) / ANTIALIASING - 0.5l) * 2.0l / fractal->width);
					long double offset_imag = (((ay + 0.5l) / ANTIALIASING - 0.5l) * 2.0l / fractal->height) * ratio;
					
					long double complex offset = offset_real + offset_imag * I;
					offset *= fractal->zoom;
					num += offset;
					
					acc += iterate(num);
				}
			}
			
			acc /= ANTIALIASING * ANTIALIASING;
		
			array[(x + line * fractal->width) * 3 + 0] = Wrap(acc * 4.34532457l); 
			array[(x + line * fractal->width) * 3 + 1] = Wrap(acc * 2.93324292l);
			array[(x + line * fractal->width) * 3 + 2] = Wrap(acc * 1.2273444l);
		}
	}
}
