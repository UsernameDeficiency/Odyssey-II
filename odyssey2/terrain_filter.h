/* Filters for terrain smoothing, designed for the square terrain maps from diamondsquare.h */
#pragma once
#include <algorithm>
#include <vector>


/* */
static void mean(std::vector<float>* arr, const int filter_width)
{
	std::vector<float> *arr_temp = arr;
	for (size_t i = filter_width; i < (double)arr->size() - 1; i++)
	{
		arr_temp->at(i) = arr->at(i - 1) / 4 + arr->at(i) / 2 + arr->at(i + 1) / 4;
	}
	for (size_t i = filter_width; i < (double)arr->size() - 1; i++)
	{
		arr->at(i) = arr_temp->at(i);
	}
}


/* */
static void median(std::vector<float>* arr, const int filter_width)
{
	float* arr_tmp = (float*)malloc(sizeof(float) * arr->size() * arr->size());
	for (size_t i = arr->size() + 2; i < (double)arr->size() * (arr->size() - 1) - 2; i++)
	{
		std::vector<float> curr{ (*arr)[i - 2], (*arr)[i - 1], (*arr)[i], (*arr)[i + 1], (*arr)[i + 2] };
		std::sort(curr.begin(), curr.end());
		arr_tmp[i] = curr.at(curr.size() / 2);
	}

	for (size_t i = arr->size() + filter_width; i < arr->size() * (arr->size() - 1) - 2; i++)
	{
		(*arr)[i] = arr_tmp[i];
	}

	for (size_t r = 0; r < arr->size(); r++)
	{
		for (size_t c = 0; r < arr->size(); c++)
		{

		}
	}
}
