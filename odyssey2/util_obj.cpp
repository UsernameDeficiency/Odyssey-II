// Based on LoadOBJ by Ingemar Ragnemalm 2005, 2008
#include "loadobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// How many error messages do you want before it stops?
//#define PI 3.141592
//
//typedef struct Mesh
//{
//	GLfloat* vertices;
//	int		vertexCount;
//	GLfloat* vertexNormals;
//	int		normalsCount; // Same as vertexCount for generated normals
//	GLfloat* textureCoords;
//	int		texCount;
//
//	int* coordIndex;
//	int* normalsIndex;
//	int* textureIndex;
//	int		coordCount; // Number of indices in each index struct
//
//	// Borders between groups
//	int* coordStarts;
//	int		groupCount;
//
//	GLfloat radius; // Enclosing sphere
//	GLfloat radiusXZ; // For cylindrical tests
//} Mesh, * MeshPtr;
//
//#define vToken			1
//#define vnToken			2
//#define vtToken			3
//#define kReal			4
//#define kInt			5
//#define tripletToken	6
//#define fToken			7
//#define crlfToken		8
//#define kEOF			9
//#define kUnknown		10
//#define gToken		11
//#define mtllibToken		12
//#define usemtlToken		13
//
//static FILE* fp;
//static int intValue[3];
//static float floatValue[3] = { 0, 0, 0 };
//static int vertCount, texCount, normalsCount, coordCount;
//static bool hasPositionIndices;
//static bool hasNormalIndices;
//static bool hasTexCoordIndices;
//static bool atLineEnd; // Helps SkipToCRLF


void ReportRerror(const char* caller, const char* name)
{
	static unsigned int draw_error_counter = 0;
	// Report error - but not more than NUM_DRAWMODEL_ERROR
	if (draw_error_counter < NUM_DRAWMODEL_ERROR)
	{
		fprintf(stderr, "%s warning: '%s' not found in shader!\n", caller, name);
		draw_error_counter++;
	}
	else if (draw_error_counter == NUM_DRAWMODEL_ERROR)
	{
		fprintf(stderr, "%s: Number of error bigger than %i. No more vill be printed.\n", caller, NUM_DRAWMODEL_ERROR);
		draw_error_counter++;
	}
}


// This code makes a lot of calls for rebinding variables just in case,
// and to get attribute locations. This is clearly not optimal, but the
// goal is stability.
void DrawModel2(Model* m, GLuint program, const char* vertexVariableName, const char* normalVariableName, const char* texCoordVariableName)
{
	if (m != NULL)
	{
		glBindVertexArray(m->vao);	// Select VAO
		glBindBuffer(GL_ARRAY_BUFFER, m->vb);

		GLint loc = glGetAttribLocation(program, vertexVariableName);
		if (loc >= 0)
		{
			glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(loc);
		}
		else
			ReportRerror("DrawModel", vertexVariableName);

		if (normalVariableName != NULL)
		{
			loc = glGetAttribLocation(program, normalVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->nb);
				glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				ReportRerror("DrawModel", normalVariableName);
		}

		// VBO for texture coordinate data NEW for 5b
		if ((m->texCoordArray != NULL) && (texCoordVariableName != NULL))
		{
			loc = glGetAttribLocation(program, texCoordVariableName);
			if (loc >= 0)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m->tb);
				glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(loc);
			}
			else
				ReportRerror("DrawModel", texCoordVariableName);
		}

		glDrawElements(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_INT, 0L);
	}
}


// Loader for inline data to Model
Model* LoadDataToModel2(
	GLfloat* vertices,
	GLfloat* normals,
	GLfloat* texCoords,
	GLuint* indices,
	int numVert,
	int numInd)
{
	Model* m = static_cast<Model*>(malloc(sizeof(Model))); // TODO: malloc -> new
	memset(m, 0, sizeof(Model));

	m->vertexArray = vertices;
	m->texCoordArray = texCoords;
	m->normalArray = normals;
	m->indexArray = indices;
	m->numVertices = numVert;
	m->numIndices = numInd;

	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vb);
	glGenBuffers(1, &m->ib);
	glGenBuffers(1, &m->nb);
	if (m->texCoordArray != NULL)
		glGenBuffers(1, &m->tb);

	ReloadModelData(m);

	return m;
}