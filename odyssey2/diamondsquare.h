/* This file implements the diamond-square algorithm for terrain generation.
 * The map must be (2^n)*(2^n) in size for some integer n.
 * This implementation is made to be added to a pre-defined heightmap describing
 * the lower frequency content, therefore the scaling for the random displacement
 * is small for both low and high, but not middle, frequencies. */
#pragma once
#include <vector>
#include <cmath>
#include <random>


 /* randnum returns a random float number between min and max, attempting to minimize rounding errors. */
float randnum(const float max, const float min)
{
	return (max - min) * (float)((double)(rand()) / (double)RAND_MAX) + min;
}


/* Returns a normally distributed random number, standard deviation stddev */
float randnum_norm(const float stddev, const float min)
{

	std::random_device rd{};
	std::mt19937 gen{ rd() };
	std::normal_distribution<float> stddist{ 0, stddev / 2 };

	return stddist(gen);
}


/* diamondsquare creates a heightmap of size width*width using the diamond square
	 algorithm with base offset weight for the random numbers.
	 filter_stop is the (HP) filter stop frequency described as a step length. */
std::vector<float> diamondsquare(const unsigned int width)
{
	const float base_weight = 6000.0f; // Base weight for randomized values in diamond-square algorithm
	const unsigned int seed = 64;
	srand(seed);
	auto terrain = new std::vector<float>((size_t)width * (size_t)width);
	
	/* Initialize corner values. Since the width for this implementation is 2^n rather than 2^n+1,
	 * the right and lower edges are "cut off" and terrain[0] wraps around. */
	float weight = base_weight;
	(*terrain)[0] = randnum(weight, -weight);

	// Iterate over step lengths.
	for (unsigned int step = width; step > 1; step /= 2) {
		// Do diamond part for current step length
		weight /= (float)sqrt(2);
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
				(*terrain)[mid] = ((*terrain)[ul] + (*terrain)[ur] + (*terrain)[ll] + (*terrain)[lr]) / 4 + randnum(weight, -weight);
			}
		}
		// Do square step for the upper and left points
		weight /= (float)sqrt(2);
		for (unsigned int row = 0; row < width; row += step) {
			for (unsigned int col = 0; col < width; col += step) {
				size_t r_left = (size_t)row + step / 2;
				size_t c_up = (size_t)col + step / 2;

				// Being lazy here and making sure all indices are in bounds, even if some will never go out of bounds.
				float mean_up = (
					(*terrain)[(((size_t)row - step / 2 + width) % width) * width + c_up] + // Above, make sure it is not negative
					(*terrain)[(size_t)row * width + (c_up - step / 2 + width) % width] + // Left, make sure it is not negative
					(*terrain)[(size_t)row * width + (c_up + step / 2) % width] + // Right
					(*terrain)[(r_left % width) * width + c_up]) / 4; // Below
				float mean_left = (
					(*terrain)[((r_left - step / 2 + width) % width) * width + col] +
					(*terrain)[r_left * width + (col - step / 2 + width) % width] +
					(*terrain)[r_left * width + c_up % width] +
					(*terrain)[((r_left + step / 2) % width) * width + col]) / 4;

				(*terrain)[(size_t)row * (size_t)width + c_up] = mean_up + randnum(weight, -weight);
				(*terrain)[r_left * width + col] = mean_left + randnum(weight, -weight);
			}
		}
	}

	return *terrain;
}
