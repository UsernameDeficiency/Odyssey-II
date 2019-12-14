/* Filters for terrain smoothing, designed for the square terrain maps from diamondsquare.h */
#pragma once
#include <algorithm>


/* */
static void mean(float arr[], const int width)
{
	for (size_t i = 1; i < (double)width * width - 1; i++)
	{
		arr[i] = arr[i - 1] / 4 + arr[i] / 2 + arr[i + 1] / 4;
	}
}


/* */
static void median(float arr[], const int width)
{
	float* arr_tmp = (float*)malloc(sizeof(float) * width * width);
	for (size_t i = width + 2; i < (double)width * (width - 1) - 2; i++)
	{
		float curr[5]{ arr[i - 2], arr[i - 1], arr[i], arr[i + 1], arr[i + 2] };
		//float curr[3]{ arr[i - 1], arr[i], arr[i + 1] };
		std::sort(curr, curr + 4);
		arr_tmp[i] = curr[3];
	}

	for (size_t i = width + 2; i < (double)width * (width - 1) - 2; i++)
	{
		arr[i] = arr_tmp[i];
	}

	for (size_t r = 0; r < width; r++)
	{
		for (size_t c = 0; r < width; c++)
		{

		}
	}
}
