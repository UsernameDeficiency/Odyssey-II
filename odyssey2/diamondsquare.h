/* This file implements the diamond-square algorithm for terrain generation.
 * The map must be (2^n)*(2^n) in size for some integer n.
 * This implementation is made to be added to a pre-defined heightmap describing
 * the lower frequency content, therefore the scaling for the random displacement
 * is small for both low and high, but not middle, frequencies. */
#pragma once
#include <cmath>
#include <iostream> // cerr


/* Settings for diamond square algorithm */
// Base weight for randomized values in diamond-square algorithm.
static const float BASE_WEIGHT = 5 * 128.0f;
// No random numbers will be added for steps with steps length shorter than LAST_STEP
static const int LAST_STEP = 8;
// Seeding for rand() in diamondsquare() (default 1).
static const unsigned int SEED = 64;


 /* randnum returns a random float number between min and max, attempting to minimize rounding errors. */
float randnum(float max, float min)
{
	return (max - min) * (float)((double)(rand()) / (double)RAND_MAX) + min;
}


/* Calculate the scaling for random numbers
	 For a "normal" implementation the amplitude is divided by sqrt(2) in each
	 step. For this implementation the cutoff variable can be set, if it is set
	 to >= 2 the amplitude for frequencies below the cutoff will be filtered out
	 for use with pre-generated heightmaps describing the low frequency content. */
static float calcweight(int step, int cutoff, float weight)
{
	if (step <= cutoff / 8.0f) { // TODO: Why divide by 8 here?
		/* High frequencies. Divide magnitude by sqrt(2) for each iteration */
		return weight / (float)sqrt(2);
	}
	else {
		/* Low frequencies, use constant filter amplitude up to cutoff frequency */
		return weight; // TODO: This is not used if no divide by 8 above
	}
}


/* diamond uses the current step length to calculate indices and do linear
	 interpolation for the diamond step */
static void diamond(float arr[], int width, int row, int col, int step, float weight, int LAST_STEP)
{
	// Indices for upper/lower right and left corners of the square area being worked on, the mean of the corner
	// values give the base displacement for the current point being calculated. Wrap-around if out of bounds.
	int ul = row * width + col; // Always in bounds
	int ur = row * width + (col + step) % width;
	int ll = ((row + step) % width) * width + col;
	int lr = ((row + step) % width) * width + (col + step) % width;
	int mid = (row + step / 2) * width + col + step / 2; // Current point being calculated (always in bounds)

	arr[mid] = (arr[ul] + arr[ur] + arr[ll] + arr[lr]) / 4; // Mean of all 4 points
	if (step >= LAST_STEP) {
		arr[mid] += randnum(weight, -weight);
	}
}


/* square uses the current step length to calculate indices and do linear
	 interpolation for the upper and left points in the square step */
static void square(float arr[], int width, int row, int col, int step, float weight, int LAST_STEP)
{
	int r_left = row + step / 2;
	int c_up = col + step / 2;

	// Being lazy here and making sure all indices are in bounds, even if some will never go out of bounds.
	float mean_up = (
		arr[((row - step / 2 + width) % width) * width + c_up] + // Above, make sure it is not negative
		arr[row * width + (c_up - step / 2 + width) % width] +   // Left, make sure it is not negative
		arr[row * width + (c_up + step / 2) % width] +           // Right
		arr[(r_left % width) * width + c_up]) / 4;               // Below
	float mean_left = (
		arr[((r_left - step / 2 + width) % width) * width + col] +
		arr[r_left * width + (col - step / 2 + width) % width] +
		arr[r_left * width + c_up % width] +
		arr[((r_left + step / 2) % width) * width + col]) / 4;

	arr[row * width + c_up] = mean_up;
	arr[r_left * width + col] = mean_left;
	if (step >= LAST_STEP) {
		arr[row * width + c_up] += randnum(weight, -weight);
		arr[r_left * width + col] += randnum(weight, -weight);
	}
}


/* diamondsquare creates a heightmap of size width*width using the diamond square
	 algorithm with base offset weight for the random numbers.
	 filter_stop is the (HP) filter stop frequency described as a step length.
	 For step lengths under LAST_STEP no random numbers will be added (LP filter). */
float* diamondsquare(const int width, const int cutoff)
{
	// Set seeding for random numbers
	srand(SEED);
	// Allocate heightmap
	float* terrain = (float*)malloc(sizeof(float) * width * width);
	
	/* Initialize corner values. Observe that, since the width for this implementation is 2^n rather than 2^n+1,
	 * the right and lower edges are "cut off" compared some other implementations and terrain[0] wraps around. */
	float weight = calcweight(width, cutoff, BASE_WEIGHT);
	terrain[0] = randnum(weight, -weight);

	// Iterate over step lengths.
	for (int step = width; step > 1; step /= 2) {
		// Do diamond part for current step length.
		weight = calcweight(step, cutoff, weight);
		for (int row = 0; row < width; row += step) {
			for (int col = 0; col < width; col += step) {
				diamond(terrain, width, row, col, step, weight, LAST_STEP);
			}
		}
		// Do square part for current step length.
		weight = calcweight(step, cutoff, weight);
		for (int row = 0; row < width; row += step) {
			for (int col = 0; col < width; col += step) {
				square(terrain, width, row, col, step, weight, LAST_STEP);
			}
		}
	}

	return terrain;
}
