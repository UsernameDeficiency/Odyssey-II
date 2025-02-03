/* Code for terrain generation and filtering */
#pragma once
#include "model.h"
#include <vector>

// Generate terrain and save it into a Model
class Terrain
{
public:
	Terrain(unsigned int world_size, float world_xz_scale);

	Model* terrain_model;
	// TODO: Move Terrain_heights from util_misc here instead?

private:
	// Build Model from generated terrain
	Model* generate_terrain(const unsigned int world_size, const float world_xz_scale);

	// Do filter_size-point moving average filtering on arr
	static void mean(std::vector<float>& arr, const unsigned int filter_size);

	// Do median filtering on arr with filter_size number of elements in each direction
	// TODO: Terrain::median is unused
	static void median(std::vector<float>& arr, const unsigned int filter_size);

	/* Create a heightmap of size width*width using the diamond square algorithm with
		base offset weight for the random numbers. */
	std::vector<float> diamondsquare(const unsigned int width);
};
