Fractal
=======

A small Mandelbrot renderer written in C.

Features:
Uses pthreads to calculate different parts of the fractal simultaneously.

The program uses the PPM file format (a simple uncompressed format) to store images. When rendering, it creates a PPM file and maps the image portion to address space using mmap, so the image isn't stored in memory. This allows for truly giant images to be created, limited only by time and hard drive space!

Todo
- Increase platform support (designed to work on just Linux at the moment)
- Implement cycle detection to speed up calculating the interior of the fractal
- Add a compressed image format, since creating most of the time we can fit the image in memory (and would vastly reduces file size!)
