/* Filters for terrain smoothing, designed for the square terrain maps from diamondsquare.h */
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>


/* mean does filter_size-point moving average filtering of arr.
	filter_size should be an odd positive integer, no sanity check done! */
static void mean(std::vector<float>* arr, const unsigned int filter_size)
{
	size_t arr_width = (size_t)sqrt(arr->size()); // width = height of terrain array
	std::vector<float>* arr_tmp = new std::vector<float>(arr->size());

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr->at(i); // Initialize average with current element
			float normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				float scale = 1 / (float)pow(2, offset); // Lower scaling for high offsets
				normalization += 2 * scale;

				// Left value
				if (col < offset)
					avg += arr->at(i - offset + arr_width) * scale; // Out of bounds, wrap to end of row
				else
					avg += arr->at(i - offset) * scale;

				// Right value
				if (col + offset >= arr_width)
					avg += arr->at(i + offset - arr_width) * scale; // Out of bounds, wrap to start of row
				else
					avg += arr->at(i + offset) * scale;
			}
			arr_tmp->at(i) = avg / normalization;
		}
	}

	// Vertical filter
	for (size_t col = 0; col < arr_width; col++) {
		for (size_t row = 0; row < arr_width; row++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr_tmp->at(i); // Initialize average with current element
			float normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				float scale = 1 / (float)pow(2, offset); // Lower scaling for high offsets
				normalization += 2 * scale;

				// Upper value
				if (row < offset)
					avg += arr_tmp->at(i - offset * arr_width + arr->size()) * scale; // Out of bounds, wrap to end of column
				else
					avg += arr_tmp->at(i - offset * arr_width) * scale;

				// Lower value
				if (row + offset >= arr_width)
					avg += arr_tmp->at(i + offset * arr_width - arr->size()) * scale; // Out of bounds, wrap to start of column
				else
					avg += arr_tmp->at(i + offset * arr_width) * scale;
			}
			arr->at(i) = avg / normalization;
		}
	}

	delete arr_tmp;
}


/* median does filter_size-point median filtering of arr.
	filter_size should be an odd positive integer, no sanity check done! */
// TODO: Performance much worse than mean()
static void median(std::vector<float>* arr, const int filter_size)
{
	size_t arr_width = (size_t)sqrt(arr->size()); // width = height of terrain array
	std::vector<float>* arr_tmp = new std::vector<float>(arr->size());
	std::vector<float> median;

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			median.push_back(arr->at(i));
			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				// Left value
				if (col < offset)
					median.push_back(arr->at(i - offset + arr_width)); // Out of bounds, wrap to end of row
				else
					median.push_back(arr->at(i - offset));

				// Right value
				if (col + offset >= arr_width)
					median.push_back(arr->at(i + offset - arr_width)); // Out of bounds, wrap to start of row
				else
					median.push_back(arr->at(i + offset));
			}
			std::sort(median.begin(), median.end());
			arr_tmp->at(i) = median.at(filter_size / 2);
			median.clear();
		}
	}

	// Vertical filter
	for (size_t col = 0; col < arr_width; col++) {
		for (size_t row = 0; row < arr_width; row++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			median.push_back(arr->at(i));
			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				// Upper value
				if (row < offset)
					median.push_back(arr_tmp->at(i - offset * arr_width + arr->size())); // Out of bounds, wrap to end of column
				else
					median.push_back(arr_tmp->at(i - offset * arr_width));

				// Lower value
				if (row + offset >= arr_width)
					median.push_back(arr_tmp->at(i + offset * arr_width - arr->size())); // Out of bounds, wrap to start of column
				else
					median.push_back(arr_tmp->at(i + offset * arr_width));
			}
			std::sort(median.begin(), median.end());
			arr->at(i) = median.at(filter_size / 2);
			median.clear();
		}
	}

	delete arr_tmp;
}
