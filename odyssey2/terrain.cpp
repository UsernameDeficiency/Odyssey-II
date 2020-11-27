/* Code for terrain generation and filtering */
#include "terrain.h"
#include <vector>
#include <cmath>
#include <algorithm>


/* mean does filter_size-point moving average filtering of arr. */
void mean(std::vector<float>& arr, const unsigned int filter_size)
{
	size_t arr_width = (size_t)sqrt(arr.size()); // width = height of terrain array
	std::vector<float> arr_tmp(arr.size());

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr[i]; // Initialize average with current element
			float normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				float scale = 1 / static_cast<float>(pow(2, offset)); // Lower scaling for high offsets
				normalization += 2 * scale;

				// Left value
				if (col < offset)
					avg += arr[i - offset + arr_width] * scale; // Out of bounds, wrap to end of row
				else
					avg += arr[i - offset] * scale;

				// Right value
				if (col + offset >= arr_width)
					avg += arr[i + offset - arr_width] * scale; // Out of bounds, wrap to start of row
				else
					avg += arr[i + offset] * scale;
			}
			arr_tmp[i] = avg / normalization;
		}
	}

	// Vertical filter
	for (size_t col = 0; col < arr_width; col++) {
		for (size_t row = 0; row < arr_width; row++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			float avg = arr_tmp[i]; // Initialize average with current element
			float normalization = 1; // Normalize calculated average

			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				float scale = 1 / static_cast<float>(pow(2, offset)); // Lower scaling for high offsets
				normalization += 2 * scale;

				// Upper value
				if (row < offset)
					avg += arr_tmp[i - offset * arr_width + arr.size()] * scale; // Out of bounds, wrap to end of column
				else
					avg += arr_tmp[i - offset * arr_width] * scale;

				// Lower value
				if (row + offset >= arr_width)
					avg += arr_tmp[i + offset * arr_width - arr.size()] * scale; // Out of bounds, wrap to start of column
				else
					avg += arr_tmp[i + offset * arr_width] * scale;
			}
			arr[i] = avg / normalization;
		}
	}
}


/* Do median filtering on arr with filter_size number of elements in each direction. */
void median(std::vector<float>& arr, const unsigned int filter_size)
{
	size_t arr_width = (size_t)sqrt(arr.size()); // width = height of terrain array
	std::vector<float> arr_tmp = std::vector<float>(arr.size());
	std::vector<float> median(4 * ((size_t)filter_size / 2) + 1);

	// Horizontal filter
	for (size_t row = 0; row < arr_width; row++) {
		for (size_t col = 0; col < arr_width; col++) {
			size_t i = row * arr_width + col; // Index of element being smoothed
			median[0] = arr[i];
			for (size_t offset = 1; offset <= filter_size / 2; offset++) {
				size_t j = 4 * (offset - 1); // Index for median vector
				// Left value
				if (col < offset)
					median[j + 1] = arr[i - offset + arr_width]; // Out of bounds, wrap to end of row
				else
					median[j + 1] = arr[i - offset];

				// Right value
				if (col + offset >= arr_width)
					median[j + 2] = arr[i + offset - arr_width]; // Out of bounds, wrap to start of row
				else
					median[j + 2] = arr[i + offset];

				// Upper value
				if (row < offset)
					median[j + 3] = arr[i - offset * arr_width + arr.size()]; // Out of bounds, wrap to end of column
				else
					median[j + 3] = arr[i - offset * arr_width];

				// Lower value
				if (row + offset >= arr_width)
					median[j + 4] = arr[i + offset * arr_width - arr.size()]; // Out of bounds, wrap to start of column
				else
					median[j + 4] = arr[i + offset * arr_width];
			}
			std::sort(median.begin(), median.end());
			arr_tmp[i] = median[median.size() / 2];
		}
	}
	arr.swap(arr_tmp);
}


/* randnum returns a random float number between min and max, attempting to minimize rounding errors. */
static float randnum(const float max, const float min)
{
	return (max - min) * static_cast<float>(rand()) / RAND_MAX + min;
}


/* diamondsquare creates a heightmap of size width*width using the diamond square
	algorithm with base offset weight for the random numbers.
	width must be (2^n)*(2^n) in size for some integer n.*/
std::vector<float> diamondsquare(const unsigned int width)
{
	float weight = 2000.0f; // Base weight for randomized values in diamond-square algorithm
	const unsigned int seed = 64;
	srand(seed);
	std::vector<float> terrain((size_t)width * width);

	/* Initialize corner values. Since the width for this implementation is 2^n rather than 2^n+1,
	 * the right and lower edges are "cut off" and terrain[0] wraps around. */
	terrain[0] = randnum(weight, -weight);

	// Iterate over step lengths.
	for (unsigned int step = width; step > 1; step /= 2) {
		// Do diamond part for current step length
		weight /= static_cast<float>(sqrt(2));
		for (unsigned int row = 0; row < width; row += step) {
			for (unsigned int col = 0; col < width; col += step) {
				// Indices for upper/lower right and left corners of the square area being worked on, the mean of the corner
				// values give the base displacement for the current point being calculated. Wrap-around if out of bounds.
				int ul = row * width + col;
				int ur = row * width + (col + step) % width;
				int ll = ((row + step) % width) * width + col;
				int lr = ((row + step) % width) * width + (col + step) % width;
				int mid = (row + step / 2) * width + col + step / 2; // Current point being calculated

				// Mean of all 4 points
				terrain[mid] = (terrain[ul] + terrain[ur] + terrain[ll] + terrain[lr]) / 4 + randnum(weight, -weight);
			}
		}
		// Do square step for the upper and left points
		weight /= static_cast<float>(sqrt(2));
		for (unsigned int row = 0; row < width; row += step) {
			for (unsigned int col = 0; col < width; col += step) {
				size_t r_left = (size_t)row + step / 2;
				size_t c_up = (size_t)col + step / 2;

				// Being lazy here and making sure all indices are in bounds, even if some will never go out of bounds.
				float mean_up = (
					terrain[(((size_t)row - step / 2 + width) % width) * width + c_up] + // Above, make sure it is not negative
					terrain[(size_t)row * width + (c_up - step / 2 + width) % width] + // Left, make sure it is not negative
					terrain[(size_t)row * width + (c_up + step / 2) % width] + // Right
					terrain[(r_left % width) * width + c_up]) / 4; // Below
				float mean_left = (
					terrain[((r_left - step / 2 + width) % width) * width + col] +
					terrain[r_left * width + (col - step / 2 + width) % width] +
					terrain[r_left * width + c_up % width] +
					terrain[((r_left + step / 2) % width) * width + col]) / 4;

				terrain[(size_t)row * (size_t)width + c_up] = mean_up + randnum(weight, -weight);
				terrain[r_left * width + col] = mean_left + randnum(weight, -weight);
			}
		}
	}

	return terrain;
}
