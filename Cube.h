#ifndef HEADER_CUBE_H
#define HEADER_CUBE_H

#include <glm/mat4x4.hpp>
#include <glad/glad.h>
#include "ShaderHelpers.h"

/* We use the following layout for vertex data */
typedef struct {
	GLfloat pos[3]; /* 3D cartesian coordinates */
	GLubyte clr[4]; /* RGBA (8bit per channel is typically enough) */
} Vertex;

static const Vertex basicCubeGeometry[] = {
	/*   X     Y     Z       R    G    B    A */
	/* front face */
	{ { -1.0, -1.0,  1.0 },{ 255,   0,   0, 255 } },
	{ { 1.0, -1.0,  1.0 },{ 192,   0,   0, 255 } },
	{ { -1.0,  1.0,  1.0 },{ 192,   0,   0, 255 } },
	{ { 1.0,  1.0,  1.0 },{ 128,   0,   0, 255 } },
	/* back face */
	{ { 1.0, -1.0, -1.0 },{ 0, 255, 255, 255 } },
	{ { -1.0, -1.0, -1.0 },{ 0, 192, 192, 255 } },
	{ { 1.0,  1.0, -1.0 },{ 0, 192, 192, 255 } },
	{ { -1.0,  1.0, -1.0 },{ 0, 128, 128, 255 } },
	/* left  face */
	{ { -1.0, -1.0, -1.0 },{ 0, 255,   0, 255 } },
	{ { -1.0, -1.0,  1.0 },{ 0, 192,   0, 255 } },
	{ { -1.0,  1.0, -1.0 },{ 0, 192,   0, 255 } },
	{ { -1.0,  1.0,  1.0 },{ 0, 128,   0, 255 } },
	/* right face */
	{ { 1.0, -1.0,  1.0 },{ 255,   0, 255, 255 } },
	{ { 1.0, -1.0, -1.0 },{ 192,   0, 192, 255 } },
	{ { 1.0,  1.0,  1.0 },{ 192,   0, 192, 255 } },
	{ { 1.0,  1.0, -1.0 },{ 128,   0, 128, 255 } },
	/* top face */
	{ { -1.0,  1.0,  1.0 },{ 0,   0, 255, 255 } },
	{ { 1.0,  1.0,  1.0 },{ 0,   0, 192, 255 } },
	{ { -1.0,  1.0, -1.0 },{ 0,   0, 192, 255 } },
	{ { 1.0,  1.0, -1.0 },{ 0,   0, 128, 255 } },
	/* bottom face */
	{ { 1.0, -1.0,  1.0 },{ 255, 255,   0, 255 } },
	{ { -1.0, -1.0,  1.0 },{ 192, 192,   0, 255 } },
	{ { 1.0, -1.0, -1.0 },{ 192, 192,   0, 255 } },
	{ { -1.0, -1.0, -1.0 },{ 128, 128,   0, 255 } },
};

/* use two triangles sharing an edge for each face */
static const GLushort basicCubeConnectivity[] = {
	0, 1, 2,  2, 1, 3,	/* front */
	4, 5, 6,  6, 5, 7,	/* back */
	8, 9,10, 10, 9,11,	/* left */
	12,13,14, 14,13,15,	/* right */
	16,17,18, 18,17,19,	/* top */
	20,21,22, 22,21,23	/* bottom */
};

/* Cube: state required for the cube. */
typedef struct {
	GLuint vbo[2];		/* vertex and index buffer names */
	GLuint vao;		/* vertex array object */
	glm::mat4 model;	/* local model transformation */

	void destroy()
	{
		glBindVertexArray(0);
		if (vao) {
			info("Cube: deleting VAO %u", vao);
			glDeleteVertexArrays(1, &vao);
			vao = 0;
		}
		if (vbo[0] || vbo[1]) {
			info("Cube: deleting VBOs %u %u", vbo[0], vbo[1]);
			glDeleteBuffers(2, vbo);
			vbo[0] = 0;
			vbo[1] = 0;
		}
	}
	void initBasic()
	{
		/* set up VAO and vertex and element array buffers */
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		info("Cube: created VAO %u", vao);

		glGenBuffers(2, vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(basicCubeGeometry), basicCubeGeometry, GL_STATIC_DRAW);
		info("Cube: created VBO %u for %u bytes of vertex data", vbo[0], (unsigned)sizeof(basicCubeGeometry));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(basicCubeConnectivity), basicCubeConnectivity, GL_STATIC_DRAW);
		info("Cube: created VBO %u for %u bytes of element data", vbo[1], (unsigned)sizeof(basicCubeConnectivity));

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, pos)));
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), BUFFER_OFFSET(offsetof(Vertex, clr)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		model = glm::mat4();
		GL_ERROR_DBG("cube initialization");

	}

} Cube;


#endif
