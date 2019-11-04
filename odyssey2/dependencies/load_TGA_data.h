#pragma once
#if defined(_WIN32)
#include <windows.h>
#include <glad/glad.h>
#else
#include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Texture struct from LoadTGA.h */
typedef struct TextureData // Create A Structure for .tga loading.
{
	GLuint texID;
	GLuint bpp; // Image color depth in bits per pixel.
	GLuint width, height; // Image size
	GLuint w, h; // "Raw" image sizes
	GLfloat texWidth, texHeight; // Unknown
	GLubyte* imageData; // Image data (up to 32 bits)
} TextureData, * TextureDataPtr;


// Taken from LoadTGA.h 2019-11-01, slight modifications only
bool LoadTGATextureData(const char* filename, TextureData* texture) // Loads a TGA file into memory
{
	GLuint i;
	GLubyte TGAuncompressedheader[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		TGAcompressedheader[12] = { 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		TGAuncompressedbwheader[12] = { 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		TGAcompressedbwheader[12] = { 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		actualHeader[12], // Used to compare TGA header
		header[6]; // First 6 useful bytes from the header
	GLuint bytesPerPixel, imageSize, temp;
	long rowSize, stepSize, bytesRead, row, rowLimit, step;
	GLubyte* rowP;
	int err{ 0 }, b;
	GLubyte rle;
	GLubyte pixelData[4];
	char flipped;

	FILE *file = fopen(filename, "rb");
	if (!file)
	{
		printf("LoadTGATextureData failed: could not open file %s\n", filename);
		return false;
	}
	else if (fread(actualHeader, 1, sizeof(actualHeader), file) != sizeof(actualHeader)) err = 1; // Are there 12 bytes to read?
	else if ((memcmp(TGAuncompressedheader, actualHeader, sizeof(TGAuncompressedheader)) != 0) &&
		(memcmp(TGAcompressedheader, actualHeader, sizeof(TGAcompressedheader)) != 0) &&
		(memcmp(TGAuncompressedbwheader, actualHeader, sizeof(TGAuncompressedheader)) != 0) &&
		(memcmp(TGAcompressedbwheader, actualHeader, sizeof(TGAcompressedheader)) != 0))
	{
		err = 2; // Does the header match what we want?
	}
	else if (fread(header, 1, sizeof(header), file) != sizeof(header)) err = 3; // If so read next 6 header bytes

	if (err)
	{
		switch (err)
		{
		case 1: printf("LoadTGATextureData failed: could not read header of %s\n", filename); break;
		case 2: printf("LoadTGATextureData failed: unsupported format in %s\n", filename); break;
		case 3: printf("LoadTGATextureData failed: could not read file %s\n", filename); break;
		}

		fclose(file); // If anything failed, close the file
		return false;
	}
	texture->width = header[1] * 256 + header[0]; // Determine the TGA Width (highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2]; // Determine the TGA Height (highbyte*256+lowbyte)
	if (texture->width <= 0 || // Is the width less than or equal to zero
		texture->height <= 0 || // Is the height less than or equal to zero
		(header[4] != 24 && header[4] != 32 && header[4] != 8)) // Is the TGA 24 or 32 bit?
	{
		fclose(file); // If anything failed, close the file
		return false;
	}
	flipped = (header[5] & 32) != 0; // Testa om flipped

	long w = 1;
	while (w < texture->width) w = w << 1;
	long h = 1;
	while (h < texture->height) h = h << 1;
	texture->texWidth = (GLfloat)texture->width / w;
	texture->texHeight = (GLfloat)texture->height / h;

	texture->bpp = header[4]; // Grab the TGA's bits per pixel (24 or 32)
	bytesPerPixel = texture->bpp / 8; // Divide by 8 to get the bytes per pixel
	imageSize = w * h * bytesPerPixel; // Calculate the memory required for the tga data
	rowSize = texture->width * bytesPerPixel; // Image memory per row
	stepSize = w * bytesPerPixel; // Memory per row
	texture->imageData = (GLubyte*)malloc(imageSize); // reserve memory to hold the tga data
	if (texture->imageData == NULL) // Does the storage memory exist?
	{
		fclose(file);
		return false;
	}

	// Set rowP and step depending on direction
	if (flipped)
	{
		step = stepSize;
		rowP = &texture->imageData[0];
		row = 0;
	}
	else
	{
		step = -stepSize;
		rowP = &texture->imageData[imageSize - stepSize];
		row = (texture->height - 1) * stepSize;
	}

	if (actualHeader[2] == 2 || actualHeader[2] == 3) // uncompressed
	{
		for (i = 0; i < texture->height; i++)
		{
			bytesRead = fread(rowP, 1, rowSize, file);
			rowP += step;
			if (bytesRead != rowSize)
			{
				free(texture->imageData);
				fclose(file);
				return false;
			}
		}
	}
	else // compressed
	{
		i = row;
		rowLimit = row + rowSize;
		do
		{
			bytesRead = fread(&rle, 1, 1, file);
			if (rle < 128)
			{ // rle+1 raw pixels
				bytesRead = fread(&texture->imageData[i], 1, (rle + 1) * bytesPerPixel, file);
				i += bytesRead;
				if (bytesRead == 0)
					i = imageSize;
			}
			else
			{ // range of rle-127 pixels with a color that follows
				bytesRead = fread(&pixelData, 1, bytesPerPixel, file);
				do
				{
					for (b = 0; b < bytesPerPixel; b++)
						texture->imageData[i + b] = pixelData[b];
					i += bytesPerPixel;
					rle = rle - 1;
				} while (rle > 127);
			}
			if (i >= rowLimit)
			{
				row = row + step; // - stepSize;
				rowLimit = row + rowSize;
				i = row;
			}
		} while (i < imageSize);
	}

	if (bytesPerPixel >= 3) // if not monochrome
	{
		for (i = 0; i < (int)imageSize; i += bytesPerPixel) // Loop through image data
		{ // Swaps the 1st and 3rd bytes ('R'ed and 'B'lue)
			temp = texture->imageData[i]; // Temporarily store the value at image data 'i'
			texture->imageData[i] = texture->imageData[i + 2]; // Set the 1st byte to the value of the 3rd byte
			texture->imageData[i + 2] = temp; // Set the 3rd byte to the value in 'temp' (1st byte value)
		}
	}

	texture->w = w;
	texture->h = h;
	fclose(file);
	return true;
}
