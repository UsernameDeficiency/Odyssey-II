/*
	Callback definitions for GLFW
*/
#pragma once
#include "main.h" // Constants and globals
#include "util_misc.h" // getPosy


// Keyboard state for key_callback/updatePhysics
enum keyEnum { KEY_FORWARD, KEY_BACKWARD, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN };
bool keyState[KEY_DOWN + 1] = { false };

// Reads keyboard state set by key_callback and updates player movement
void updatePhysics()
{
	if (keyState[KEY_FORWARD])
		camera.ProcessKeyboard(CAM_FORWARD, deltaTime);
	if (keyState[KEY_BACKWARD])
		camera.ProcessKeyboard(CAM_BACKWARD, deltaTime);
	if (keyState[KEY_LEFT])
		camera.ProcessKeyboard(CAM_LEFT, deltaTime);
	if (keyState[KEY_RIGHT])
		camera.ProcessKeyboard(CAM_RIGHT, deltaTime);

	if (camera.flying)
	{
		if (keyState[KEY_UP])
			camera.ProcessKeyboard(CAM_UP, deltaTime);
		if (keyState[KEY_DOWN])
			camera.ProcessKeyboard(CAM_DOWN, deltaTime);
	}
	else
	{
		// Move player to terrain height, 2.0f padding ensures arrays stay in bound during interpolation (?)
		if (camera.Position.x < 0)
			camera.Position.x = 0;
		else if (camera.Position.x > world_size * world_xz_scale - 2.0f)
			camera.Position.x = world_size * world_xz_scale - 2.0f;
		if (camera.Position.z < 0)
			camera.Position.z = 0;
		else if (camera.Position.z > world_size * world_xz_scale - 2.0f)
			camera.Position.z = world_size * world_xz_scale - 2.0f;

		const float new_y_pos = getPosy(camera.Position.x, camera.Position.z, mTerrain->vertexArray, &terrainTex) + camera.height;
		const float swim_height = sea_y_pos + camera.height / 3;

		// Make sure player does not drown.
		if (new_y_pos > swim_height)
			camera.Position.y = new_y_pos;
		else
			camera.Position.y = swim_height;
	}
}


// Read keyboard input, do bounds checking for player and moving objects
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			keyState[KEY_FORWARD] = true;
		else if (action == GLFW_RELEASE)
			keyState[KEY_FORWARD] = false;
	}
	else if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			keyState[KEY_BACKWARD] = true;
		else if (action == GLFW_RELEASE)
			keyState[KEY_BACKWARD] = false;
	}
	else if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			keyState[KEY_LEFT] = true;
		else if (action == GLFW_RELEASE)
			keyState[KEY_LEFT] = false;
	}
	else if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			keyState[KEY_RIGHT] = true;
		else if (action == GLFW_RELEASE)
			keyState[KEY_RIGHT] = false;
	}

	// Move faster is left shift held down
	else if (key == GLFW_KEY_LEFT_SHIFT)
	{
		if (action == GLFW_PRESS)
			camera.MovementSpeed = CAM_SPEED * 8;
		else if (action == GLFW_RELEASE)
			camera.MovementSpeed = CAM_SPEED;
	}

	// Zoom by lowering fov
	else if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			camera.projection = glm::perspective(glm::radians(camera.fov / 4), (GLfloat)window_w / (GLfloat)window_h, VP_NEAR, 1.5f * VP_FAR);
			camera.MouseSensitivity = CAM_SENSITIVITY / 3.0f;
		}
		else if (action == GLFW_RELEASE)
		{
			camera.projection = glm::perspective(glm::radians(camera.fov), (GLfloat)window_w / (GLfloat)window_h, VP_NEAR, VP_FAR);
			camera.MouseSensitivity = CAM_SENSITIVITY;
		}
	}

	// Crouch
	else if (key == GLFW_KEY_C)
	{
		if (action == GLFW_PRESS)
		{
			camera.height = CAM_HEIGHT / 2.0f;
			camera.MovementSpeed = CAM_SPEED * 0.4;
		}
		else if (action == GLFW_RELEASE)
		{
			camera.height = CAM_HEIGHT;
			camera.MovementSpeed = CAM_SPEED;
		}
	}

	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// Toggle fog
	else if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		drawFog = !drawFog;
		terrainShader->use();
		terrainShader->setInt("drawFog", drawFog);
		skyboxShader->use();
		skyboxShader->setInt("drawFog", drawFog);
		waterShader->use();
		waterShader->setInt("drawFog", drawFog);
	}

	// Toggle flight/walk mode
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		camera.flying = !camera.flying;

	if (camera.flying) // Player flying
	{
		if (key == GLFW_KEY_E)
		{
			if (action == GLFW_PRESS)
				keyState[KEY_UP] = true;
			else if (action == GLFW_RELEASE)
				keyState[KEY_UP] = false;
		}
		else if (key == GLFW_KEY_Q)
		{
			if (action == GLFW_PRESS)
				keyState[KEY_DOWN] = true;
			else if (action == GLFW_RELEASE)
				keyState[KEY_DOWN] = false;
		}
	}
	else // Player walking
		keyState[KEY_DOWN] = keyState[KEY_UP] = false;
}


// Called on mouse movement
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	float xoffset = xpos - mouseLastX;
	float yoffset = mouseLastY - ypos; // reversed since y-coordinates go from bottom to top

	mouseLastX = xpos;
	mouseLastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


/* fb_size_callback is called when the window is resized and updates the projection matrix and viewport size */
static void fb_size_callback(GLFWwindow* window, int width, int height)
{
	window_w = width;
	window_h = height;
	camera.projection = glm::perspective(glm::radians(camera.fov), (GLfloat)width / (GLfloat)height, VP_NEAR, VP_FAR);
	glViewport(0, 0, width, height);
}


/* error_callback is called on each glfw error, upon which it displays an error code 
   and description */
static void error_callback(int code, const char* description)
{
	std::cerr << "GLFW error " << code << " : " << description << std::endl;
}
