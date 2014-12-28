#include "Fractal.h"

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

#define ANTIALIASING 1
#define MAX_ITER 15000

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

struct Fractal *Fractal_Create(char *file_name, int width, int height, long double complex position, long double zoom)
{
	struct Fractal *fractal = malloc(sizeof (struct Fractal));
	if (!fractal) {
		return NULL;
	}
	
	fractal->width = width;
	fractal->height = height;
	fractal->position = position;
	fractal->zoom = zoom;	
	fractal->line = 0;
	
	pthread_mutex_init(&fractal->lock, NULL);
	
	// Open file
	fractal->fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0666);
	
	if (fractal->fd == -1) {
		perror("Error opening file");

		free(fractal);	
		return NULL;
	}
	
	// Write PPM image header
	char *header;
	int header_length = asprintf(&header, "P6\n%d %d\n255\n", width, height);

	if (header_length == -1) {
		if (close(fractal->fd) == -1) {
			// Should probably handle this error
		}
		
		free(fractal);
		return NULL;
	}
	
	write(fractal->fd, header, header_length);
	
	free(header);
	
	// Resize file to full-size
	fractal->array_size = width * height * 3;
	fractal->file_size = header_length + fractal->array_size;
	
	lseek(fractal->fd, fractal->array_size - 1, SEEK_CUR);
	
	write(fractal->fd, "", 1);
	
	// Map file to memory
	fractal->map = mmap(NULL, fractal->file_size, PROT_WRITE, MAP_SHARED, fractal->fd, 0);
	
	if (fractal->map == MAP_FAILED) {
		perror("Error mapping file");
		return NULL;
	}
	
	fractal->array = fractal->map + header_length;

	return fractal;
}

void Fractal_Destroy(struct Fractal *fractal)
{
	if(munmap(fractal->map, fractal->file_size) == -1) {
		perror("Failed to unmap file");
	}
	
	close(fractal->fd);

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
	
	long double escape = 2.0l;
	long double escape_squared = escape * escape;
	
	long double ratio = fractal->height / fractal->width;
	
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
		
			fractal->array[(x + line * fractal->width) * 3] = Wrap(co * 4.34532457l); 
			fractal->array[(x + line * fractal->width) * 3 + 1] = Wrap(co * 2.93324292l);
			fractal->array[(x + line * fractal->width) * 3 + 2] = Wrap(co * 1.2273444l);
		}
	}
}
