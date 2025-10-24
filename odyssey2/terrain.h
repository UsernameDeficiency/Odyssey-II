/* Code for terrain generation and filtering */
#pragma once
#include "model.h"
#include <vector>

// Generate terrain and save it into a Model
class Terrain
{
public:
	Terrain(const unsigned int world_size, const float world_xz_scale);

	Model* terrain_model;

	// Lowest point in generated terrain
	float min_height{ FLT_MAX };

	// Highest point in generated terrain
	float max_height{ -FLT_MAX };

	// y coordinate of sea level
	float sea_height{};

	// Size of terrain in number of vertices along one side (world is square)
	const unsigned int world_size;

	// Scale of terrain in x and z directions
	const float world_xz_scale;

private:
	// Build Model from generated terrain
	Model* generate_terrain();

	// Do filter_size-point moving average filtering on arr
	static void mean(std::vector<float>& arr, const unsigned int filter_size);

	// Do median filtering on arr with filter_size number of elements in each direction
	static void median(std::vector<float>& arr, const unsigned int filter_size);

	/* Create a heightmap of size width*width using the diamond square algorithm with
		base offset weight for the random numbers. */
	std::vector<float> diamondsquare(const unsigned int width);
};
