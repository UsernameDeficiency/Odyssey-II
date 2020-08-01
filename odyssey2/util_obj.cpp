// Based on LoadOBJ by Ingemar Ragnemalm 2005, 2008
#include "util_obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// TODO: Possibly remove this function
void ReportRerror(const char* caller, const char* name)
{
	unsigned int num_drawmodel_error = 8;
	static unsigned int draw_error_counter = 0;
	// Report error - but not more than num_drawmodel_error
	if (draw_error_counter < num_drawmodel_error)
	{
		fprintf(stderr, "%s warning: '%s' not found in shader!\n", caller, name);
		draw_error_counter++;
	}
	else if (draw_error_counter == num_drawmodel_error)
	{
		fprintf(stderr, "%s: Number of error bigger than %i. No more vill be printed.\n", caller, num_drawmodel_error);
		draw_error_counter++;
	}
}


// This code makes a lot of calls for rebinding variables just in case,
// and to get attribute locations. This is clearly not optimal, but the
// goal is stability.
void DrawModel(Model* m, GLuint program, const char* vertexVariableName, 
	const char* normalVariableName, const char* texCoordVariableName)
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


// Called from LoadDataToModel. Useful by its own when the model changes on CPU
// VAO and VBOs must already exist!
void ReloadModelData(Model* m)
{
	glBindVertexArray(m->vao);

	// VBO for vertex data
	glBindBuffer(GL_ARRAY_BUFFER, m->vb);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m->numVertices) * 3 * sizeof(GLfloat), m->vertexArray, GL_STATIC_DRAW);

	// VBO for normal data
	glBindBuffer(GL_ARRAY_BUFFER, m->nb);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m->numVertices) * 3 * sizeof(GLfloat), m->normalArray, GL_STATIC_DRAW);

	// VBO for texture coordinate data
	if (m->texCoordArray != NULL)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m->tb);
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m->numVertices) * 2 * sizeof(GLfloat), m->texCoordArray, GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->numIndices * sizeof(GLuint), m->indexArray, GL_STATIC_DRAW);
}


// Loader for inline data to Model
Model* LoadDataToModel(GLfloat* vertices, GLfloat* normals,
	GLfloat* texCoords, GLuint* indices, int numVert, int numInd)
{
	Model* m = new Model;
	//memset(m, 0, sizeof(Model));

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
