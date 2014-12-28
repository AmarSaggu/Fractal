#include "Fractal.h"

#include <pthread.h>

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#define NUM 3

int main()
{
	//long double complex position = -9.156725634575431530763915102604629272702E-01l + -2.764022557666976021281036574985851235422E-01 * I;
	
	long double complex position = -1.401141611950751478232223636650572367261E+00 + 6.134219667583451483105922099438475822671E-06l * I;
	
	//long double zoom = powl(0.95, 550.0l);
	
	//long double zoom = 9.998146882276323755254436873673271488478E-16l;
	long double zoom = 1.0l;
	
	
	for (int frame = 0; frame < 1000; frame++) {
	
		if (frame > 400) {
		//long double zoom  = powl(0.5l, frame - 50);
		
		char *filename;
		asprintf(&filename, "Image-%u.ppm", frame + 1);

		struct Fractal *fractal = Fractal_Create(filename, 1920, 1080, position, zoom);

		pthread_t thread[NUM];

		for(int i = 0; i < NUM; ++i) {
			 pthread_create(&thread[ i ], NULL, Fractal_Render, fractal);
		}
		
		for(int i = 0; i < NUM; ++i) {
			pthread_join(thread[i], NULL);
		}

		puts("");

		Fractal_Destroy(fractal);

		free(filename);
		}
		
		zoom *= 0.95l;
	}
	
	return 0;
}
