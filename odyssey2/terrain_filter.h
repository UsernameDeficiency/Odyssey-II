/* Filters for terrain smoothing, designed for the square terrain maps from diamondsquare.h */
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>


/* mean does filter_size-point moving average filtering of arr. */
static void mean(std::vector<float>& arr, const unsigned int filter_size)
{
	size_t arr_width = (size_t)sqrt(arr.size()); // width = height of terrain array
	std::vector<float> arr_tmp(arr.size());

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr.at(i); // Initialize average with current element
			float normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				float scale = 1 / (float)pow(2, offset); // Lower scaling for high offsets
				normalization += 2 * scale;

				// Left value
				if (col < offset)
					avg += arr.at(i - offset + arr_width) * scale; // Out of bounds, wrap to end of row
				else
					avg += arr.at(i - offset) * scale;

				// Right value
				if (col + offset >= arr_width)
					avg += arr.at(i + offset - arr_width) * scale; // Out of bounds, wrap to start of row
				else
					avg += arr.at(i + offset) * scale;
			}
			arr_tmp.at(i) = avg / normalization;
		}
	}

	// Vertical filter
	for (size_t col = 0; col < arr_width; col++) {
		for (size_t row = 0; row < arr_width; row++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr_tmp.at(i); // Initialize average with current element
			float normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				float scale = 1 / (float)pow(2, offset); // Lower scaling for high offsets
				normalization += 2 * scale;

				// Upper value
				if (row < offset)
					avg += arr_tmp.at(i - offset * arr_width + arr.size()) * scale; // Out of bounds, wrap to end of column
				else
					avg += arr_tmp.at(i - offset * arr_width) * scale;

				// Lower value
				if (row + offset >= arr_width)
					avg += arr_tmp.at(i + offset * arr_width - arr.size()) * scale; // Out of bounds, wrap to start of column
				else
					avg += arr_tmp.at(i + offset * arr_width) * scale;
			}
			arr.at(i) = avg / normalization;
		}
	}
}


/* Do median filtering on arr with filter_size number of elements in each direction. */
static void median(std::vector<float>& arr, const unsigned int filter_size)
{
	size_t arr_width = (size_t)sqrt(arr.size()); // width = height of terrain array
	std::vector<float> arr_tmp = std::vector<float>(arr.size());
	std::vector<float> median(4 * ((size_t)filter_size / 2) + 1);

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			median.at(0) = arr.at(i);
			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				size_t j = 4 * (offset - 1); // Index for median vector
				// Left value
				if (col < offset)
					median.at(j + 1) = arr.at(i - offset + arr_width); // Out of bounds, wrap to end of row
				else
					median.at(j + 1) = arr.at(i - offset);

				// Right value
				if (col + offset >= arr_width)
					median.at(j + 2) = arr.at(i + offset - arr_width); // Out of bounds, wrap to start of row
				else
					median.at(j + 2) = arr.at(i + offset);

				// Upper value
				if (row < offset)
					median.at(j + 3) = arr.at(i - offset * arr_width + arr.size()); // Out of bounds, wrap to end of column
				else
					median.at(j + 3) = arr.at(i - offset * arr_width);

				// Lower value
				if (row + offset >= arr_width)
					median.at(j + 4) = arr.at(i + offset * arr_width - arr.size()); // Out of bounds, wrap to start of column
				else
					median.at(j + 4) = arr.at(i + offset * arr_width);
			}
			std::sort(median.begin(), median.end());
			arr_tmp.at(i) = median.at(median.size() / 2);
		}
	}
	arr.swap(arr_tmp);
}
