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

void *Fractal_Render(void *arg)
{
	struct Fractal *fractal = arg;
	
	uint8_t *array = fractal->image->array;
	
	long double escape = 1000.0l;
	long double escape_squared = escape * escape;
	
	long double ratio = fractal->height;
	ratio /= fractal->width;
	
	long double log_escape = logl(escape);
	long double log_two = logl(2.0l);
	
	for (;;) {
		// Lock
		pthread_mutex_lock(&fractal->lock);
		
		int line = fractal->line;
	
		if (line >= fractal->height) {
			// No more rendering to perform
			pthread_mutex_unlock(&fractal->lock);
			return NULL;
		} else {
			// Fetch the next horizontal line to render
			fractal->line++;
			
			printf("\r%d / %d", fractal->line, fractal->height);
			fflush(stdout);
			
			pthread_mutex_unlock(&fractal->lock);
		}
	
		for (unsigned int x = 0; x < fractal->width; ++x) {
			long double num2_real = ((x + 0.5l) / fractal->width - 0.5l) * 2.0l;
			long double num2_imag = ((line + 0.5l) / fractal->height - 0.5l) * 2.0l * ratio;
			long double complex num2 = num2_real + num2_imag * I;
			num2 *= fractal->zoom;
			num2 += fractal->position;
			
			long double co = 0.0l;
			
			for (unsigned int yy = 0; yy < ANTIALIASING; ++yy) {
				for(unsigned int xx = 0; xx < ANTIALIASING; ++xx) {
					//long double complex num = ( x - fractal->width / 2.0l + 0.5l ) * 2.0l * fractal->zoom + ( line - fractal->height / 2.0l + 0.5l ) * 2.0l * fractal->zoom * I + fractal->position;
	
					long double num_real = (((xx + 0.5l) / ANTIALIASING - 0.5l) * 2.0l / fractal->width);
					long double num_imag = (((yy + 0.5l) / ANTIALIASING - 0.5l) * 2.0l / fractal->height) * ratio;
					long double complex num = num_real + num_imag * I;
					num *= fractal->zoom;
					num + fractal->position;
					
					num += num2;
	
					long double complex c = num;
					
					for (unsigned int i = 0; i < MAX_ITER; ++i) {
						num *= num;
						num += c;
						
						if (unlikely(creall(num) * creall(num) + cimagl(num) * cimagl(num) >= escape_squared)) {
							// Perform smoothing
							long double moo = i + 1.0l - (logl(logl(cabs(num)) / log_escape) / log_two);
							
							moo = logl(moo);
							moo *= 175.0l;
							co += moo;

							break;
						}
					}
				}
			}
			
			co /= ANTIALIASING * ANTIALIASING;
		
			//co /= 3.35423;
		
			array[(x + line * fractal->width) * 3] = Wrap(co * 4.34532457l); 
			array[(x + line * fractal->width) * 3 + 1] = Wrap(co * 2.93324292l);
			array[(x + line * fractal->width) * 3 + 2] = Wrap(co * 1.2273444l);
		}
	}
}
