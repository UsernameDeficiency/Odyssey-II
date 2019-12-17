/* Filters for terrain smoothing, designed for the square terrain maps from diamondsquare.h */
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>


/* mean does filter_width-point moving average filtering of arr.
	filter_width should be an odd positive integer, no sanity check done! */
static void mean(std::vector<float>* arr, const unsigned int filter_width)
{
	size_t arr_width = sqrt(arr->size()); // width = height of terrain array
	std::vector<float>* arr_tmp = new std::vector<float>(arr->size());

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr->at(i); // Initialize average with current element
			size_t normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_width / 2; offset++) {
				size_t scale = pow(2, offset); // Lower scaling for high offsets
				normalization += 2 / scale;

				// Left value
				if (col < offset)
					avg += arr->at(i - offset + arr_width) / scale; // Out of bounds, wrap to end of row
				else
					avg += arr->at(i - offset) / scale;

				// Right value
				if (col + offset >= arr_width)
					avg += arr->at(i + offset - arr_width) / scale; // Out of bounds, wrap to start of row
				else
					avg += arr->at(i + offset) / scale;
			}
			arr_tmp->at(i) = avg / normalization;
		}
	}

	// Vertical filter
	for (size_t col = 0; col < arr_width; col++) {
		for (size_t row = 0; row < arr_width; row++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr_tmp->at(i); // Initialize average with current element
			size_t normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_width / 2; offset++) {
				size_t scale = pow(2, offset); // Lower scaling for high offsets
				normalization += 2 / scale;

				// Upper value
				if (row < offset)
					avg += arr_tmp->at(i - offset * arr_width + arr->size()) / scale; // Out of bounds, wrap to end of column
				else
					avg += arr_tmp->at(i - offset * arr_width) / scale;

				// Lower value
				if (row + offset >= arr_width)
					avg += arr_tmp->at(i + offset * arr_width - arr->size()) / scale; // Out of bounds, wrap to start of column
				else
					avg += arr_tmp->at(i + offset * arr_width) / scale;
			}
			arr->at(i) = avg / normalization;
		}
	}

	delete arr_tmp;
}


/* */
static void median(std::vector<float>* arr, const int filter_width)
{
	std::vector<float>* arr_tmp = new std::vector<float>(arr->size());

	for (size_t i = arr->size() + 2; i < (double)arr->size() * (arr->size() - 1) - 2; i++)
	{
		std::vector<float> curr{ (*arr)[i - 2], (*arr)[i - 1], (*arr)[i], (*arr)[i + 1], (*arr)[i + 2] };
		std::sort(curr.begin(), curr.end());
		arr_tmp->at(i) = curr.at(curr.size() / 2);
	}

	for (size_t i = arr->size() + filter_width; i < arr->size() * (arr->size() - 1) - 2; i++)
	{
		(*arr)[i] = arr_tmp->at(i);
	}

	delete arr_tmp;
}
