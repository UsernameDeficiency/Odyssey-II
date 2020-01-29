/* This file implements the diamond-square algorithm for terrain generation.
 * The map must be (2^n)*(2^n) in size for some integer n.
 * This implementation is made to be added to a pre-defined heightmap describing
 * the lower frequency content, therefore the scaling for the random displacement
 * is small for both low and high, but not middle, frequencies. */
#pragma once
#include <vector>
#include <cmath>
#include <random>


/* Settings for diamond square algorithm */
// Base weight for randomized values in diamond-square algorithm.
static const float BASE_WEIGHT = 6000.0f;
static const unsigned int SEED = 64;


 /* randnum returns a random float number between min and max, attempting to minimize rounding errors. */
float randnum(float max, float min)
{
	return (max - min) * (float)((double)(rand()) / (double)RAND_MAX) + min;
}


/* Returns a normally distributed random number, standard deviation stddev */
float randnum_norm(float stddev, float min)
{

	std::random_device rd{};
	std::mt19937 gen{ rd() };
	std::normal_distribution<float> stddist{ 0, stddev / 2 };

	return stddist(gen);
}


/* diamond uses the current step length to calculate indices and do linear
	 interpolation for the diamond step */
static void diamond(std::vector<float> *arr, unsigned int width, unsigned int row, unsigned int col, unsigned int step, float weight)
{
	// Indices for upper/lower right and left corners of the square area being worked on, the mean of the corner
	// values give the base displacement for the current point being calculated. Wrap-around if out of bounds.
	int ul = row * width + col; // Always in bounds
	int ur = row * width + (col + step) % width;
	int ll = ((row + step) % width) * width + col;
	int lr = ((row + step) % width) * width + (col + step) % width;
	int mid = (row + step / 2) * width + col + step / 2; // Current point being calculated (always in bounds)

	// Mean of all 4 points
	(*arr)[mid] = ((*arr)[ul] + (*arr)[ur] + (*arr)[ll] + (*arr)[lr]) / 4 + randnum(weight, -weight);
}


/* square uses the current step length to calculate indices and do linear
	 interpolation for the upper and left points in the square step */
static void square(std::vector<float> *arr, unsigned int width, unsigned int row, unsigned int col, unsigned int step, float weight)
{
	size_t r_left = (size_t)row + step / 2;
	size_t c_up = (size_t)col + step / 2;

	// Being lazy here and making sure all indices are in bounds, even if some will never go out of bounds.
	float mean_up = (
		(*arr)[(((size_t)row - step / 2 + width) % width) * width + c_up] + // Above, make sure it is not negative
		(*arr)[(size_t)row * width + (c_up - step / 2 + width) % width] + // Left, make sure it is not negative
		(*arr)[(size_t)row * width + (c_up + step / 2) % width] + // Right
		(*arr)[(r_left % width) * width + c_up]) / 4; // Below
	float mean_left = (
		(*arr)[((r_left - step / 2 + width) % width) * width + col] +
		(*arr)[r_left * width + (col - step / 2 + width) % width] +
		(*arr)[r_left * width + c_up % width] +
		(*arr)[((r_left + step / 2) % width) * width + col]) / 4;

	(*arr)[(size_t)row * (size_t)width + c_up] = mean_up + randnum(weight, -weight);
	(*arr)[r_left * width + col] = mean_left + randnum(weight, -weight);
}


/* diamondsquare creates a heightmap of size width*width using the diamond square
	 algorithm with base offset weight for the random numbers.
	 filter_stop is the (HP) filter stop frequency described as a step length. */
std::vector<float> diamondsquare(const unsigned int width)
{
	// Set seeding for random numbers
	srand(SEED);
	// Allocate heightmap
	auto terrain = new std::vector<float>((size_t)width * (size_t)width);
	
	/* Initialize corner values. Since the width for this implementation is 2^n rather than 2^n+1,
	 * the right and lower edges are "cut off" and terrain[0] wraps around. */
	float weight = BASE_WEIGHT;
	(*terrain)[0] = randnum(weight, -weight);

	// Iterate over step lengths.
	for (unsigned int step = width; step > 1; step /= 2) {
		// Do diamond part for current step length
		weight /= (float)sqrt(2);
		for (unsigned int row = 0; row < width; row += step) {
			for (unsigned int col = 0; col < width; col += step) {
				diamond(terrain, width, row, col, step, weight);
			}
		}
		// Do square part for current step length.
		weight /= (float)sqrt(2);
		for (unsigned int row = 0; row < width; row += step) {
			for (unsigned int col = 0; col < width; col += step) {
				square(terrain, width, row, col, step, weight);
			}
		}
	}

	return *terrain;
}
