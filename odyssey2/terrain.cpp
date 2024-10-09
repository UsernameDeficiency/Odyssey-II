/* Code for terrain generation and filtering */
#include "terrain.h"
#include "io.h"
#include "model.h"
#include "util_misc.h"
#include "glm/vec3.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>


// TODO: Move into Terrain?
extern struct Terrain_heights terrain_struct; // Used by generate_terrain to set heights for water and snow


Terrain::Terrain(unsigned int world_size, float world_xz_scale)
{
	terrain_model = generate_terrain(world_size, world_xz_scale);
}


/* Build Model from generated terrain. */
Model* Terrain::generate_terrain(const unsigned int world_size, const float world_xz_scale)
{
	const float tex_scale{ 1.0f / 4.0f }; // Scaling of texture coordinates

	// Build procedural terrain and smooth result
	std::vector<float> proc_terrain = diamondsquare(world_size);
	mean(proc_terrain, 5);

	const size_t vertex_count = static_cast<size_t>(world_size) * world_size;
	const size_t triangle_count = static_cast<size_t>(world_size - 1) * static_cast<size_t>(world_size - 1) * 2ull;
	// Since vertices are ordered in a cartesian grid the x and y positions might not be needed?
	// It might be possible to use integer types for some or all of these values
	std::vector<GLfloat> vertex_array(vertex_count * 3);
	std::vector<GLfloat> normal_array(vertex_count * 3);
	std::vector<GLfloat> tex_coord_array(vertex_count * 2);
	std::vector<GLuint> index_array(triangle_count * 3);

	// Fill vertex, texture coordinate and index array
	for (unsigned int x = 0; x < world_size; x++) {
		for (unsigned int z = 0; z < world_size; z++) {
			size_t index = x + z * static_cast<size_t>(world_size);
			float y = proc_terrain[index];
			if (y < terrain_struct.min_height)
				terrain_struct.min_height = y;
			if (y > terrain_struct.max_height)
				terrain_struct.max_height = y;

			vertex_array[index * 3] = x * world_xz_scale;
			vertex_array[index * 3 + 1] = y;
			vertex_array[index * 3 + 2] = z * world_xz_scale;

			/* Scaled texture coordinates. */
			tex_coord_array[index * 2 + 0] = static_cast<float>(x) * tex_scale;
			tex_coord_array[index * 2 + 1] = static_cast<float>(z) * tex_scale;

			if ((x != world_size - 1) && (z != world_size - 1)) {
				index = (x + z * static_cast<size_t>(world_size - 1)) * 6;
				// Triangle 1
				index_array[index] = x + z * world_size;
				index_array[index + 1] = x + (z + 1) * world_size;
				index_array[index + 2] = x + 1 + z * world_size;
				// Triangle 2
				index_array[index + 3] = x + 1 + z * world_size;
				index_array[index + 4] = x + (z + 1) * world_size;
				index_array[index + 5] = x + 1 + (z + 1) * world_size;
			}
		}
	}

	// Calculate normals (cross product of two vectors along current triangle)
	const size_t offset = static_cast<size_t>(world_size) * 3;
	for (unsigned int x = 0; x < world_size; x++) {
		for (unsigned int z = 0; z < world_size; z++) {
			size_t index = (x + z * static_cast<size_t>(world_size)) * 3;
			// Initialize normals along edges to pointing straight up
			if (x == 0 || (x == world_size - 1) || z == 0 || (z == world_size - 1)) {
				normal_array[index] = 0.0;
				normal_array[index + 1] = 1.0;
				normal_array[index + 2] = 0.0;
			}
			// Inside edges, here the required indices are in bounds
			else {
				glm::vec3 p0(vertex_array[index + offset], vertex_array[index + 1 + offset], vertex_array[index + 2 + offset]);
				glm::vec3 p1(vertex_array[index - offset], vertex_array[index - offset + 1], vertex_array[index - offset + 2]);
				glm::vec3 p2(vertex_array[index - 3], vertex_array[index - 2], vertex_array[index - 1]);
				glm::vec3 a(p1 - p0);
				glm::vec3 b(p2 - p0);
				glm::vec3 normal = glm::cross(a, b);

				normal_array[index] = normal.x;
				normal_array[index + 1] = normal.y;
				normal_array[index + 2] = normal.z;
			}
		}
	}

	// Create Model and upload to GPU (formerly LoadModelData)
	Model* m = new Model(std::move(vertex_array),
		static_cast<GLsizei>(vertex_count), static_cast<GLsizei>(triangle_count) * 3);

	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vb);
	glGenBuffers(1, &m->ib);
	glGenBuffers(1, &m->nb);
	glGenBuffers(1, &m->tb);

	// ReloadModelData() functionality below
	const GLsizeiptr vert_size = m->numVertices * sizeof(GLfloat);
	glBindVertexArray(m->vao);
	glBindBuffer(GL_ARRAY_BUFFER, m->vb);
	glBufferData(GL_ARRAY_BUFFER, vert_size * 3, m->vertexArray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->numIndices * sizeof(GLuint), index_array.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m->nb);
	glBufferData(GL_ARRAY_BUFFER, vert_size * 3, normal_array.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m->tb);
	glBufferData(GL_ARRAY_BUFFER, vert_size * 2, tex_coord_array.data(), GL_STATIC_DRAW);

	return m;
}


/* mean does filter_size-point moving average filtering of arr. */
void Terrain::mean(std::vector<float>& arr, const unsigned int filter_size)
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
void Terrain::median(std::vector<float>& arr, const unsigned int filter_size)
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


/* diamondsquare creates a heightmap of size width*width using the diamond square algorithm with base offset weight
	for the random numbers. width must be (2^n)*(2^n) in size for some integer n.*/
std::vector<float> Terrain::diamondsquare(const unsigned int width)
{
	float weight{ stof(read_string_from_ini("weight", "2000.0f")) }; // Base weight for randomized values in diamond-square algorithm
	const unsigned int seed{ stoul(read_string_from_ini("seed", "64")) };
	srand(seed);
	std::vector<std::vector<float>> terrain{ (size_t)width, std::vector<float>((size_t)width) };

	/* Initialize corner values. Since the width for this implementation is 2^n rather than 2^n+1,
	 * the right and lower edges are "cut off" and terrain[0] wraps around. */
	terrain[0][0] = randnum(weight, -weight);

	// Iterate over step lengths.
	for (unsigned int step = width; step > 1; step /= 2) {
		// Do diamond step for current step length
		weight /= 2;
		for (size_t row = 0; row < width; row += step) {
			for (size_t col = 0; col < width; col += step) {
				// Index upper/lower right and left corners of the square area being worked on, the mean of the corner
				// values give the base displacement for the current point being calculated. Wrap-around if out of bounds.
				terrain[row + step / 2][col + step / 2] = (
					terrain[row][col] +
					terrain[row][(col + step) % width] +
					terrain[(row + step) % width][col] +
					terrain[(row + step) % width][(col + step) % width]) / 4 + randnum(weight, -weight);
			}
		}
		// Do square step for the upper and left points
		for (size_t row = 0; row < width; row += step) {
			for (size_t col = 0; col < width; col += step) {
				size_t r_left = row + step / 2;
				size_t c_up = col + step / 2;

				// Being lazy here and making sure all indices are in bounds, even if some will never go out of bounds.
				float mean_up = (
					terrain[(row - step / 2 + width) % width][c_up] + // Above, make sure it is not negative
					terrain[row][(c_up - step / 2 + width) % width] + // Left, make sure it is not negative
					terrain[row][(c_up + step / 2) % width] + // Right
					terrain[(r_left % width)][c_up]) / 4; // Below
				float mean_left = (
					terrain[(r_left - step / 2 + width) % width][col] +
					terrain[r_left][(col - step / 2 + width) % width] +
					terrain[r_left][c_up % width] +
					terrain[(r_left + step / 2) % width][col]) / 4;

				terrain[row][c_up] = mean_up + randnum(weight, -weight);
				terrain[r_left][col] = mean_left + randnum(weight, -weight);
			}
		}
	}

	// Flatten vector
	std::vector<float> temp;
	for (const auto& vec : terrain)
		temp.insert(temp.end(), vec.begin(), vec.end());

	return temp;
}
