#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "glfw.h"
#include "app.h"

const int appWidth = 1024;
const int appHeight = 576;
static bool fullScreen = false;
static int benchmarkLength = 600;

bool setupWindow(int, int, bool);
static bool running;
static double t0;
static int mx0, my0;
static Application *app;


std::string extractAppPath(char *fullPath)
{
#ifdef __APPLE__
	std::string s(fullPath);
	for (int i = 0; i < 4; ++i)
		s = s.substr(0, s.rfind("/"));
	return s + "/../";
#else
	const std::string s(fullPath);
	if (s.find("/") != std::string::npos)
		return s.substr(0, s.rfind("/")) + "/";
	else if (s.find("\\") != std::string::npos)
		return s.substr(0, s.rfind("\\")) + "\\";
	else
		return "";
#endif
}


int windowCloseListener()
{
	running = false;
	return 0;
}


void keyPressListener(int key, int action)
{
	if (!running) return;

	if (action == GLFW_PRESS)
	{
		int width = appWidth, height = appHeight;

		switch (key)
		{
		case GLFW_KEY_ESC:
			running = false;
			break;
		case GLFW_KEY_F1:
			app->release();
			glfwCloseWindow();

			fullScreen = !fullScreen;

			if (fullScreen)
			{
				GLFWvidmode mode;
				glfwGetDesktopMode(&mode);

				float aspect = mode.Width / (float)mode.Height;
				if ((int)(aspect * 100) == 133 || (int)(aspect * 100) == 125)  
				{
					width = 1280; height = 1024;
				}
				else if ((int)(aspect * 100) == 177)                           
				{
					width = 1280; height = 720;
				}
				else if ((int)(aspect * 100) == 160)                          
				{
					width = 1280; height = 800;
				}
				else                                                            
				{
					width = mode.Width; height = mode.Height;
				}
			}

			if (!setupWindow(width, height, fullScreen))
			{
				glfwTerminate();
				exit(-1);
			}

			app->init();
			app->resize(width, height);
			t0 = glfwGetTime();
			break;
		}
	}
}


void mouseMoveListener(int x, int y)
{
	if (!running)
	{
		mx0 = x; my0 = y;
		return;
	}

	app->mouseMoveEvent((float)(x - mx0), (float)(my0 - y));
	mx0 = x; my0 = y;
}


bool setupWindow(int width, int height, bool fullscreen)
{
	if (!glfwOpenWindow(width, height, 8, 8, 8, 8, 24, 8, fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW))
	{
		glfwTerminate();
		return false;
	}

	glfwSwapInterval(0);

	glfwSetWindowCloseCallback(windowCloseListener);
	glfwSetKeyCallback(keyPressListener);
	glfwSetMousePosCallback(mouseMoveListener);

	return true;
}


int main(int argc, char** argv)
{
	glfwInit();
	glfwEnable(GLFW_STICKY_KEYS);
	if (!setupWindow(appWidth, appHeight, fullScreen)) return -1;

	bool benchmark = false;
	if (argc > 1 && strcmp(argv[1], "-bm") == 0)
	{
		benchmark = true;
		glfwDisable(GLFW_AUTO_POLL_EVENTS);
	}

	app = new Application(extractAppPath(argv[0]));
	if (!fullScreen) glfwSetWindowTitle(app->getTitle());

	if (!app->init())
	{
		glfwCloseWindow();
		glfwOpenWindow(800, 16, 8, 8, 8, 8, 24, 8, GLFW_WINDOW);
		glfwSetWindowTitle("Unable to initalize engine - Make sure you have an OpenGL 2.0 compatible graphics card");
		double startTime = glfwGetTime();
		while (glfwGetTime() - startTime < 5.0) {}  // Sleep

		std::cout << "Unable to initalize engine" << std::endl;
		std::cout << "Make sure you have an OpenGL 2.0 compatible graphics card";
		glfwTerminate();
		return -1;
	}
	app->resize(appWidth, appHeight);

	glfwDisable(GLFW_MOUSE_CURSOR);

	int frames = 0;
	float fps = 30.0f;
	t0 = glfwGetTime();
	running = true;

	while (running)
	{
		++frames;
		if (!benchmark && frames >= 3)
		{
			double t = glfwGetTime();
			fps = frames / (float)(t - t0);
			if (fps < 5) fps = 30;  
			frames = 0;
			t0 = t;
		}

		for (int i = 0; i < 320; ++i)
			app->setKeyState(i, glfwGetKey(i) == GLFW_PRESS);
		app->keyStateHandler();

		app->mainLoop(benchmark ? 60 : fps);
		glfwSwapBuffers();

		if (benchmark && frames == benchmarkLength) break;
	}

	glfwEnable(GLFW_MOUSE_CURSOR);

	if (benchmark)
	{
		double avgFPS = benchmarkLength / (glfwGetTime() - t0);
		char title[256];
		sprintf(title, "Average FPS: %.2f", avgFPS);
		glfwCloseWindow();
		glfwOpenWindow(800, 16, 8, 8, 8, 8, 24, 8, GLFW_WINDOW);
		glfwSetWindowTitle(title);
		double startTime = glfwGetTime();
		while (glfwGetTime() - startTime < 5.0) {}  // Sleep
	}

	app->release();
	delete app;
	glfwTerminate();

	return 0;
}
