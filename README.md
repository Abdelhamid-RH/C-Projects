# C-Projects
This repository contains some C programs I make as I study and explore various file systems and formats.

## `filter`
A program that applies image filters to 24-bit bitmap photos. It starts by reading the 54-byte metadata header to extract image dimensions, then transforms the 2D pixel array by:
* Averaging RGB values for the grayscale filter
* Reversing pixel column order for thr reflection filter
* Applying a 3x3 convolution matrix for the box blur filter