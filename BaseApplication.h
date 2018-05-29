#ifndef HEADER_BASEAPPLICATION_H
#define HEADER_BASEAPPLICATION_H

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glad/glad.h>
#include "Cube.h"

/* CubeApp: We encapsulate all of our application state in this struct.
* We use a single instance of this object (in main), and set a pointer to
* this as the user-defined pointer for GLFW windows. That way, we have access
* to this data structure without ever using global variables.
*/
typedef struct {
	/* the window and related state */
	GLFWwindow *win;
	int width, height;
	unsigned int flags;

	/* timing */
	double timeCur, timeDelta;
	double avg_frametime;
	double avg_fps;

	/* keyboard handling */
	bool pressedKeys[GLFW_KEY_LAST + 1];
	bool releasedKeys[GLFW_KEY_LAST + 1];

	/* the cube we want to render */
	Cube cube;

	/* the OpenGL state we need for the shaders */
	GLuint program;		/* shader program */
	GLint locProjection;
	GLint locModelView;
	GLint locTime;
	GLint locCameraPos;

	/*  the gloabal transformation matrices */
	glm::mat4 projection;
	glm::mat4 view;

	/* Create and compile the shaders and link them to a program and qeury
	* the locations of the uniforms we use.
	* Returns true if successfull and false in case of an error. */
	bool initShaders(const char *vs, const char *fs)
	{
		destroyShaders();
		program = programCreateFromFiles(vs, fs);
		if (program == 0)
			return false;

		locProjection = glGetUniformLocation(program, "projection");
		locModelView = glGetUniformLocation(program, "modelView");
		locTime = glGetUniformLocation(program, "time");
		locCameraPos = glGetUniformLocation(program, "cameraPosition");
		info("program %u: location for \"projection\" uniform: %d", program, locProjection);
		info("program %u: location for \"modelView\" uniform: %d", program, locModelView);
		info("program %u: location for \"time\" uniform: %d", program, locTime);
		info("program %u: location for \"locCameraPos\" uniform: %d", program, locCameraPos);

		return true;
	}

	/* Initialize the Cube Application.
	* This will initialize the app object, create a windows and OpenGL context
	* (via GLFW), initialize the GL function pointers via GLEW and initialize
	* the cube.
	* Returns true if successfull or false if an error occured. */
	bool initBaseApp(int w, int h, const char* title, GLFWframebuffersizefun callback_Resize, GLFWkeyfun callback_Keyboard)
	{
		int i;

		/* Initialize the app structure */
		win = NULL;
		width = w;
		height = h;
		flags = 1;
		avg_frametime = -1.0;
		avg_fps = -1.0;

		for (i = 0; i <= GLFW_KEY_LAST; i++)
			pressedKeys[i] = releasedKeys[i] = false;

		cube.vbo[0] = cube.vbo[1] = cube.vao = 0;
		program = 0;

		/* initialize GLFW library */
		info("initializing GLFW");
		if (!glfwInit()) {
			warn("Failed to initialze GLFW");
			return false;
		}

		/* request a OpenGL 3.2 core profile context */
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		/* create the window and the gl context */
		info("creating window and OpenGL context");
		win = glfwCreateWindow(w, h, title, NULL, NULL);
		if (!win) {
			warn("failed to get window with OpenGL 3.2 core context");
			return false;
		}

		/* store a pointer to our application context in GLFW's window data.
		* This allows us to access our data from within the callbacks */
		glfwSetWindowUserPointer(win, this);
		/* register our callbacks */
		glfwSetFramebufferSizeCallback(win, callback_Resize);
		glfwSetKeyCallback(win, callback_Keyboard);

		/* make the context the current context (of the current thread) */
		glfwMakeContextCurrent(win);

		/* ask the driver to enable synchronizing the buffer swaps to the
		* VBLANK of the display. Depending on the driver and the user's
		* setting, this may have no effect. But we can try... */
		glfwSwapInterval(1);

		/* initialize glad,
		* this will load all OpenGL function pointers
		*/
		info("initializing glad");
		if (!gladLoadGL()) {
			warn("failed to intialize OpenGL functions via glad");
			return false;
		}

		/* initialize the GL context */
		initGLState();
		cube.initBasic();

		/* initialize the timer */
		timeCur = glfwGetTime();

		return true;
	}
	
	/* Destroy all GL objects related to the shaders. */
	void destroyShaders()
	{
		if (program) {
			info("deleting program %u", program);
			glDeleteProgram(program);
			program = 0;
		}
	}

	/* Clean up: destroy everything the cube app still holds */
	void destroy()
	{
		if (flags) {
			cube.destroy();
			destroyShaders();
			if (win)
				glfwDestroyWindow(win);
			glfwTerminate();
		}
	}

} BaseApplication;



#endif
