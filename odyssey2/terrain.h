/* Filters for terrain smoothing, designed for the square terrain maps from diamondsquare.h */
#pragma once
#include <vector>

/* mean does filter_size-point moving average filtering of arr. */
void mean(std::vector<float>& arr, const unsigned int filter_size);

/* Do median filtering on arr with filter_size number of elements in each direction. */
void median(std::vector<float>& arr, const unsigned int filter_size);

/* diamondsquare creates a heightmap of size width*width using the diamond square
	algorithm with base offset weight for the random numbers. */
std::vector<float> diamondsquare(const unsigned int width);
