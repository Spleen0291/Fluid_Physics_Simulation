#pragma once
#define USE_CPP_IOSTREAM 1
#if USE_CPP_IOSTREAM
	#include <iostream>
	#include <iomanip>
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

class Window {
public:
	GLFWwindow* win;
	int width;
	int height;
	float aspectRatio = 1.0f; 
	static unsigned int vbo;
	static unsigned int vao;
	static std::vector<float> recData;
	Window(int w, int h, bool waitVSnyc = true);
	static void drawBoundary(int object_Location, int color_Location);
};