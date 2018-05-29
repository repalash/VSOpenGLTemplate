#define APP_TITLE "Hello, cube!"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ShaderHelpers.h"

#include "BaseApplication.h"
#include "Cube.h"

/* This function is registered as the framebuffer size callback for GLFW,
 * so GLFW will call this whenever the window is resized. */
static void callback_Resize(GLFWwindow *win, int w, int h)
{
	BaseApplication *app=(BaseApplication*)glfwGetWindowUserPointer(win);
	info("new framebuffer size: %dx%d pixels",w,h);

	/* store curent size for later use in the main loop */
	app->width=w;
	app->height=h;
}

/* This function is registered as the keyboard callback for GLFW, so GLFW
 * will call this whenever a key is pressed. */
static void callback_Keyboard(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	/* The shaders we load on the number keys. We always load a combination of
	 * a vertex and a fragment shader. */
	static const char* shaders[][2]={
		/* 0 */ {"shaders/minimal.vs.glsl", "shaders/minimal.fs.glsl"},
		/* 1 */ {"shaders/color.vs.glsl", "shaders/color.fs.glsl"},
		/* 2 */ {"shaders/cut.vs.glsl", "shaders/cut.fs.glsl"},
		/* 3 */ {"shaders/wobble.vs.glsl", "shaders/color.fs.glsl"},
		/* 4 */ {"shaders/experimental.vs.glsl", "shaders/experimental.fs.glsl"},
		/* placeholders for additional shaders */
		/* 5 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 6 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 7 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 8 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"},
		/* 9 */ {"shaders/yourshader.vs.glsl", "shaders/yourshader.fs.glsl"}
	};

	BaseApplication *app=(BaseApplication*)glfwGetWindowUserPointer(win);

	if (key < 0 || key > GLFW_KEY_LAST) {
		warn("invalid key code %d?!",key);
		return;
	}

	if (action == GLFW_RELEASE) {
		app->pressedKeys[key] = false;
	} else {
		if (!app->pressedKeys[key]) {
			/* handle certain keys */
			if (key >= '0' && key <= '9') {
				app->initShaders(shaders[key - '0'][0], shaders[key - '0'][1]);
			} else {
				switch (key) {
					case GLFW_KEY_ESCAPE:
						glfwSetWindowShouldClose(win,1);
						break;
				}
			}
		}
		app->pressedKeys[key] = true;
	}
}

/****************************************************************************
 * DRAWING FUNCTION                                                         *
 ****************************************************************************/

/* The main drawing function. This is responsible for drawing the next frame,
 * it is called in a loop as long as the application runs */
static void
displayFunc(BaseApplication *app)
{
	/* set up projection and view matrices
	 * (we do this every frame although we do not strictly have to do it,
	 * as those matrixes do never change in our small example) */
	app->projection=glm::perspective( glm::half_pi<float>(), (float)app->width/(float)app->height, 0.1f, 10.0f);
	app->view=glm::translate(glm::vec3(0.0f, 0.0f, -4.0f));

	/* rotate the cube */
	app->cube.model = glm::rotate(app->cube.model, (float)(glm::half_pi<double>() * app->timeDelta), glm::vec3(0.8f, 0.6f, 0.1f));
	/* combine model and view matrices to the modelView matrix our
	 * shader expects */
	glm::mat4 modelView = app->view * app->cube.model;

	/* set the viewport (might have changed since last iteration) */
	glViewport(0, 0, app->width, app->height);

	/* real drawing starts here drawing */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* clear the buffers */

	/* use the program and update the uniforms */
	glUseProgram(app->program);
	glUniformMatrix4fv(app->locProjection, 1, GL_FALSE, glm::value_ptr(app->projection));
	glUniformMatrix4fv(app->locModelView, 1, GL_FALSE, glm::value_ptr(modelView));
	glUniform1f(app->locTime, app->timeCur);

	/* draw the cube */
	glBindVertexArray(app->cube.vao);
	glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	/* "unbind" the VAO and the program. We do not have to do this.
	 * OpenGL is a state machine. The last binings will stay effective
	 * until we actively change them by binding something else. */
	glBindVertexArray(0);
	glUseProgram(0);

	/* finished with drawing, swap FRONT and BACK buffers to show what we
	 * have rendered */
	glfwSwapBuffers(app->win);

	/* In DEBUG builds, we also check for GL errors in the display
	 * function, to make sure no GL error goes unnoticed. */
	GL_ERROR_DBG("display function");
}

/****************************************************************************
 * MAIN LOOP                                                                *
 ****************************************************************************/

/* The main loop of the application. This will call the display function
 *  until the application is closed. This function also keeps timing
 *  statistics. */
static void mainLoop(BaseApplication *app)
{
	unsigned int frame=0,frames_total=0;
	double start_time=glfwGetTime();
	double last_time=start_time;

	info("entering main loop");
	while (!glfwWindowShouldClose(app->win)) {
		/* update the current time and time delta to last frame */
		double now=glfwGetTime();
		app->timeDelta = now - app->timeCur;
		app->timeCur = now;

		/* update FPS estimate at most once every second */
		double elapsed = app->timeCur - last_time;
		if (elapsed >= 1.0) {
			char WinTitle[80];
			app->avg_frametime=1000.0 * elapsed/(double)frame;
			app->avg_fps=(double)frame/elapsed;
			last_time=app->timeCur;
			frames_total += frame;
			frame=0;
			/* update window title */
			mysnprintf(WinTitle, sizeof(WinTitle), APP_TITLE "   /// AVG: %4.2fms/frame (%.1ffps)", app->avg_frametime, app->avg_fps);
			glfwSetWindowTitle(app->win, WinTitle);
			info("frame time: %4.2fms/frame (%.1ffps)",app->avg_frametime, app->avg_fps);
		}

		/* call the display function */
		displayFunc(app);
		frame++;

		/* This is needed for GLFW event handling. This function
		 * will call the registered callback functions to forward
		 * the events to us. */
		glfwPollEvents();
	}
	frames_total += frame;
	info("left main loop\n%u frames rendered in %.1fs seconds == %.1ffps",
		frames_total, (app->timeCur-start_time),
		(double)frames_total/(app->timeCur-start_time) );
}

/****************************************************************************
 * PROGRAM ENTRY POINT                                                      *
 ****************************************************************************/

int main (int argc, char **argv)
{
	BaseApplication app;	/* the cube application stata stucture */

	if (app.initBaseApp(800, 600, APP_TITLE, callback_Resize, callback_Keyboard)) {
		if (!app.initShaders("shaders/raymarch.vs.glsl", "shaders/raymarch.fs.glsl")) {
			warn("something wrong with our shaders...");
		}
		else {
			/* initialization succeeded, enter the main loop */
			mainLoop(&app);
		}
	}

	/* clean everything up */
	app.destroy();
	return 0;
}

