cmake_minimum_required(VERSION 2.8)

project(Fractal)

set(CMAKE_C_FLAGS "-std=gnu99 -lm -Ofast -ffast-math -fassociative-math -lpthread -march=native -mtune=native")

add_executable(Fractal Source/Main.c Source/Fractal.c Source/Image.c)
