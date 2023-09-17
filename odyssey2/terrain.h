#pragma once
#include <vector>
#include "model.h" // Model. 

/* Generates terrain and saves it into a Model */
class Terrain
{
public:
	Terrain(unsigned int world_size, float world_xz_scale);

	Model* terrain_model;
	// TODO: Move Terrain_heights from util_misc here instead?

private:
	/* Build Model from generated terrain */
	Model* generate_terrain(const unsigned int world_size, const float world_xz_scale);

	/* mean does filter_size-point moving average filtering of arr. */
	void mean(std::vector<float>& arr, const unsigned int filter_size);

	/* Do median filtering on arr with filter_size number of elements in each direction. */
	void median(std::vector<float>& arr, const unsigned int filter_size);

	/* diamondsquare creates a heightmap of size width*width using the diamond square
		algorithm with base offset weight for the random numbers. */
	std::vector<float> diamondsquare(const unsigned int width);
};
