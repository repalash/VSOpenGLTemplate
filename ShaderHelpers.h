#ifndef SHADER_HELPERS_H
#define SHADER_HELPERS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
* UTILITY FUNCTIONS: warning output, gl error checking                     *
****************************************************************************/

/* Print a info message to stdout, use printf syntax. */
static void info(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
	fputc('\n', stdout);
}

/* Print a warning message to stderr, use printf syntax. */
static void warn(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputc('\n', stderr);
}

/* Check for GL errors. If ignore is not set, print a warning if an error was
* encountered.
* Returns GL_NO_ERROR if no errors were set. */
static GLenum getGLError(const char *action, bool ignore = false, const char *file = NULL, const int line = 0)
{
	GLenum e, err = GL_NO_ERROR;

	do {
		e = glGetError();
		if ((e != GL_NO_ERROR) && (!ignore)) {
			err = e;
			if (file)
				fprintf(stderr, "%s:", file);
			if (line)
				fprintf(stderr, "%d:", line);
			warn("GL error 0x%x at %s", (unsigned)err, action);
		}
	} while (e != GL_NO_ERROR);
	return err;
}


/* helper macros:
* define GL_ERROR_DBG() to be present only in DEBUG builds. This way, you can
* add error checks at strategic places without influencing the performance
* of the RELEASE build */
#ifdef NDEBUG
#define GL_ERROR_DBG(action) (void)0
#else
#define GL_ERROR_DBG(action) getGLError(action, false, __FILE__, __LINE__)
#endif

/* define BUFFER_OFFSET to specify offsets inside VBOs */
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/* define mysnprintf to be either snprintf (POSIX) or sprintf_s (MS Windows) */
#ifdef WIN32
#define mysnprintf sprintf_s
#else
#define mysnprintf snprintf
#endif

/****************************************************************************
* UTILITY FUNCTIONS: print information about the GL context                *
****************************************************************************/

/* Print info about the OpenGL context */
static void printGLInfo()
{
	/* get infos about the GL implementation */
	info("OpenGL: %s %s %s",
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER),
		glGetString(GL_VERSION));
	info("OpenGL Shading language: %s",
		glGetString(GL_SHADING_LANGUAGE_VERSION));
}

/* List all supported GL extensions */
static void listGLExtensions()
{
	GLint num = 0;
	GLuint i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num);
	info("GL extensions supported: %d", num);
	if (num < 1) {
		return;
	}

	for (i = 0; i<(GLuint)num; i++) {
		const GLubyte *ext = glGetStringi(GL_EXTENSIONS, i);
		if (ext) {
			info("  %s", ext);
		}
	}
}

/****************************************************************************
* SHADER COMPILATION AND LINKING                                           *
****************************************************************************/

/* Print the info log of the shader compiler/linker.
* If program is true, obj is assumed to be a program object, otherwise, it
* is assumed to be a shader object.
*/
static void printInfoLog(GLuint obj, bool program)
{
	char log[16384];
	if (program) {
		glGetProgramInfoLog(obj, 16384, 0, log);
	}
	else {
		glGetShaderInfoLog(obj, 16384, 0, log);
	}
	log[16383] = 0;
	fprintf(stderr, "%s\n", log);
}

/* Create a new shader object, attach "source" as source string,
* and compile it.
* Returns the name of the newly created shader object, or 0 in case of an
* error.
*/
static  GLuint shaderCreateAndCompile(GLenum type, const GLchar *source)
{
	GLuint shader = 0;
	GLint status;

	shader = glCreateShader(type);
	info("created shader object %u", shader);
	glShaderSource(shader, 1, (const GLchar**)&source, NULL);
	info("compiling shader object %u", shader);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		warn("Failed to compile shader");
		printInfoLog(shader, false);
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}


/* Create a new shader object by loading a file, and compile it.
* Returns the name of the newly created shader object, or 0 in case of an
* error.
*/
static  GLuint shaderCreateFromFileAndCompile(GLenum type, const char *filename)
{

	info("loading shader file '%s'", filename);
	FILE *file = fopen(filename, "rt");
	if (!file) {
		warn("Failed to open shader file '%s'", filename);
		return 0;
	}
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	GLchar *source = (GLchar*)malloc(size + 1);
	if (!source) {
		warn("Failed to allocate memory for shader file '%s'", filename);
		fclose(file);
		return 0;
	}
	fseek(file, 0, SEEK_SET);
	source[fread(source, 1, size, file)] = 0;
	fclose(file);

	GLuint shader = shaderCreateAndCompile(type, source);
	free(source);
	return shader;
}



/* Create a program by linking a vertex and fragment shader object. The shader
* objects should already be compiled.
* Returns the name of the newly created program object, or 0 in case of an
* error.
*/
static GLuint programCreate(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint program = 0;
	GLint status;

	program = glCreateProgram();
	info("created program %u", program);

	if (vertex_shader)
		glAttachShader(program, vertex_shader);
	if (fragment_shader)
		glAttachShader(program, fragment_shader);

	/* hard-code the attribute indices for the attributeds we use */
	glBindAttribLocation(program, 0, "pos");
	glBindAttribLocation(program, 1, "nrm");
	glBindAttribLocation(program, 2, "clr");
	glBindAttribLocation(program, 3, "tex");

	/* hard-code the color number of the fragment shader output */
	glBindFragDataLocation(program, 0, "color");

	/* finally link the program */
	info("linking program %u", program);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		warn("Failed to link program!");
		printInfoLog(program, true);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}


/* Create a program object directly from vertex and fragment shader source
* files.
* Returns the name of the newly created program object, or 0 in case of an
* error.
*/
static GLenum programCreateFromFiles(const char *vs, const char *fs)
{
	GLuint id_vs = shaderCreateFromFileAndCompile(GL_VERTEX_SHADER, vs);
	GLuint id_fs = shaderCreateFromFileAndCompile(GL_FRAGMENT_SHADER, fs);
	GLuint program = programCreate(id_vs, id_fs);
	/* Delete the shader objects. Since they are still in use in the
	* program object, OpenGL will not destroy them internally until
	* the program object is destroyed. The caller of this function
	* does not need to care about the shader objects at all. */
	info("destroying shader object %u", id_vs);
	glDeleteShader(id_vs);
	info("destroying shader object %u", id_fs);
	glDeleteShader(id_fs);
	return program;
}

/* Initialize the global OpenGL state. This is called once after the context
* is created. */
static void initGLState()
{
	printGLInfo();
	listGLExtensions();

	/* we set these once and never change them, so there is no need
	* to set them during the main loop */
	glEnable(GL_DEPTH_TEST);

	/* We do not enable backface culling, since the "cut" shader works
	* best when one can see through the cut-out front faces... */
	//glEnable(GL_CULL_FACE);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
}

#endif // !SHADER_HELPERS_H

