#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <stdio.h>
#include <stdlib.h>
#include <glad/include/glad/glad.h>
#include <glad/src/glad.c>
#include <GLFW/glfw3.h>
#include "math.h"
#define MIKA 1
#if MIKA
#include "Client.h"
#include "cppincludes.h"
#endif
#include "inputs.h"
#include "renderer.h"
#include "camera.h"
#include "game.h"
static void glfw_error_callback(int e, const char *d) {
	printf("Error %d: %s\n", e, d);
}

GLFWwindow* init_window() 
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE  );
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwSwapInterval(0);
	GLFWwindow* window = glfwCreateWindow(SCREENWIDHT,SCREENHEIGHT, "Tabula Rasa", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, SCREENWIDHT, SCREENHEIGHT);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetErrorCallback(glfw_error_callback);
	return window;
}

int main(int argc,char* argv[])
{
	(void)argc;(void)argv;
#if MIKA
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
	btSequentialImpulseConstraintSolver* solver = new  btSequentialImpulseConstraintSolver;
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,
			overlappingPairCache, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -4.f, 0));
	Client* connection = new Client("127.0.0.1", 60000, "Loyalisti"); //Create new connection to server;
	connection->OpenConnection(); //Let attempt to open it;
	int a, w, d, s = 0;
	//TODO(mika) vaihda noista inteist� keyn yhteen inttiin
	connection->SetVar(PLAYER_INPUT, std::vector<int*>{&w, &a, &s, &d});

#endif
	GLFWwindow* window = init_window();
	Input inputs;
	init_inputs(&inputs);
	glfwSetWindowUserPointer(window,&inputs);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
	Renderer renderer = init_renderer();
	Camera camera = get_camera(
			{0.f,0.f,-3.f},
			0,-90,
			90.f,	
			(float)SCREENWIDHT / (float)SCREENHEIGHT
			);
	float deltaAngle = 1.f * deg_to_rad;
	quaternion rotaxis({90.f,0.f,90.f},deltaAngle);
	quaternion rotation;
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		{
			break;
		}

		glfwPollEvents();
#if MIKA
		connection->Update();

		if(key_down(Key::KEY_A)) {  a = 1; }
		else { a = 0; }
		if(key_down(Key::KEY_D)) { d = 1; } 
		else { d = 0; }
		if(key_down(Key::KEY_W)) { w = 1; } 
		else { w = 0; }
		if (key_down(Key::KEY_S)) { s = 1; }
		else { s = 0; }

		update_camera(&camera);

		if (connection->playerPos.size() > connection->playerSlot)
		{
			camera.position = vec3(connection->playerPos[connection->playerSlot].getX(), connection->playerPos[connection->playerSlot].getY()+6,connection->playerPos[connection->playerSlot].getZ());
		}
#endif

		if(key_pressed(Key::KEY_E)) { LOG("e pressed\n"); }
		else if (key_down(Key::KEY_E)) { LOG("e down"); }

		update_keys(&inputs);
		int display_w,display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClear(GL_COLOR_BUFFER_BIT);
		rotation *= rotaxis;

		/*Test of network handled cubes*/

#if MIKA
		for (int i = 0; i < connection->cubePos.size(); i++)
		{
			vec3 pos = vec3(connection->cubePos[i].getX(),connection->cubePos[i].getY(),connection->cubePos[i].getZ());
			vec3 rots = vec3(connection->cubeRot[i].getX(),connection->cubeRot[i].getY(),connection->cubeRot[i].getZ());
			quaternion rot = quaternion(rots, connection->cubeRot[i].getW());
			render_cube(&renderer, pos, 1, rot, {255,0,0,255});
		}
		for (int i = 0; i < connection->playerPos.size(); i++)
		{
			vec3 pos = vec3(connection->playerPos[i].getX(),connection->playerPos[i].getY(),connection->playerPos[i].getZ());
			vec3 rots = vec3(connection->playerRot[i].getX(),connection->playerRot[i].getY(),connection->playerRot[i].getZ());
			quaternion rot = quaternion(rots, connection->playerRot[i].getW());
			if (i == connection->playerSlot)
			{
				printf("%f , %f, %f \n", pos.x, pos.y, pos.z);
			}
			render_cube(&renderer, pos, 2, rot, {255,255,0,255});
		}

#endif
		/*End of the test*/
		render_cube(&renderer,
				{0,0,0},
				{1.f,1.f,1.f},
				rotation,
				//{0,0,0,1.f},
				{255,255,255,255});
		render_cube(&renderer,
				{0,-3.f,0},
				{10.f,1.f,10.f},
				{1,0,0,0},
				//{0,0,0,1.f},
				{0,0,255,255});
		render(&renderer,camera);
		glfwSwapBuffers(window);
	}
	glfwTerminate();
#if MIKA
	connection->CloseConnection(); //Close connection to server;
	delete connection; //Hakai the connecsjioon;
#endif
	printf("bye!\n");
	return 0;
}

