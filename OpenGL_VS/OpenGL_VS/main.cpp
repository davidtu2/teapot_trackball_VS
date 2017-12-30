/*
David Tu
david.tu2@csu.fullerton.edu

A toy program which renders a teapot and two light sources.
This program uses quaternions to make rotations on a trackball.
The trackball implementation uses shoemaker's method
*/

#include <tuple>
#include <cstdlib>
#include <cstdio>
//#include <sys/time.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "GLFWApp.h"
#include "GLSLShader.h"
#include "glut_teapot.h"
#include "SpinningLight.h"
#include "Camera.h"
#include "UtahTeapot.h"
#include "utilities.h"
#include "Quaternion.h"

void msglVersion(void) {
	fprintf(stderr, "OpenGL Version Information:\n");
	fprintf(stderr, "\tVendor: %s\n", glGetString(GL_VENDOR));
	fprintf(stderr, "\tRenderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stderr, "\tOpenGL Version: %s\n", glGetString(GL_VERSION));
	fprintf(stderr, "\tGLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

class TeapotTrackballApp : public GLFWApp {
private:
	float rotationDelta;
	glm::vec3 centerPosition;
	Camera mainCamera;
	glm::mat4 modelViewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 normalMatrix;
	GLSLProgram shaderProgram;
	SpinningLight light0;
	SpinningLight light1;
	UtahTeapot teapot;

	//Used to keep track of the last rotation so that the teapot won't reset
	glm::mat4 rotation;
	glm::mat4 newRotation;
	glm::vec3 prevCoord;
	glm::vec3 currentCoord;
	GLfloat angle;
	glm::vec3 axis;
	//Used to indicate to the program when I am holding the mouse
	bool held = false;

	//Variables to set uniform params for lighting fragment shader 
	unsigned int uModelViewMatrix;
	unsigned int uProjectionMatrix;
	unsigned int uNormalMatrix;
	unsigned int uLight0_position;
	unsigned int uLight0_color;
	unsigned int uLight1_position;
	unsigned int uLight1_color;
	unsigned int uAmbient;
	unsigned int uDiffuse;
	unsigned int uSpecular;
	unsigned int uShininess;

public:
	TeapotTrackballApp(int argc, char* argv[]) :
		GLFWApp(argc, argv, std::string("Teapot Vision").c_str(), 600, 600) {}

	void initCenterPosition() {
		centerPosition = glm::vec3(0.0, 0.0, 0.0);
	}

	void initTeapot() {
		Material* m = new Material(glm::vec4(0.2, 0.2, 0.2, 1.0), glm::vec4(0.0, 1.0, 1.0, 1.0),
			glm::vec4(1.0, 1.0, 1.0, 1.0), 100.0);
		teapot = UtahTeapot(centerPosition, 1.0, m);
	}

	void initCamera() {
		mainCamera = Camera(glm::vec3(0.0, 0.0, 20.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), 45.0, 1.0, 100.0);
	}

	void initRotationDelta() {
		rotationDelta = deg2rad(1.0);
	}

	void initLights() {
		glm::vec3 color0(1.0, 1.0, 1.0);
		glm::vec3 position0(0.0, 50.0, 50.0);
		glm::vec3 color1(1.0, 1.0, 1.0);
		glm::vec3 position1(0.0, 50.0, -50.0);
		light0 = SpinningLight(color0, position0, centerPosition);
		light1 = SpinningLight(color1, position1, centerPosition);
	}

	void initRotation() {
		rotation = glm::mat4();
	}

	bool begin() {
		msglError();
		initCenterPosition();
		initTeapot();
		initCamera();
		initRotationDelta();
		initLights();
		initRotation();

		//Load shader programs
		const char* vertexShaderSource = "blinn_phong.vert.glsl";
		const char* fragmentShaderSource = "blinn_phong.frag.glsl";
		FragmentShader fragmentShader(fragmentShaderSource);
		VertexShader vertexShader(vertexShaderSource);
		shaderProgram.attach(vertexShader);
		shaderProgram.attach(fragmentShader);
		shaderProgram.link();
		shaderProgram.activate();
		printf("Shader program built from %s and %s.\n", vertexShaderSource, fragmentShaderSource);
		if (shaderProgram.isActive()) {
			printf("Shader program is loaded and active with id %d.\n", shaderProgram.id());
		}
		else {
			printf("Shader program is not active, id: %d\n.", shaderProgram.id());
		}

		//Set up uniform variables for the shader program
		uModelViewMatrix = glGetUniformLocation(shaderProgram.id(), "modelViewMatrix");
		uProjectionMatrix = glGetUniformLocation(shaderProgram.id(), "projectionMatrix");
		uNormalMatrix = glGetUniformLocation(shaderProgram.id(), "normalMatrix");
		uLight0_position = glGetUniformLocation(shaderProgram.id(), "light0_position");
		uLight0_color = glGetUniformLocation(shaderProgram.id(), "light0_color");
		uLight1_position = glGetUniformLocation(shaderProgram.id(), "light1_position");
		uLight1_color = glGetUniformLocation(shaderProgram.id(), "light1_color");
		uAmbient = glGetUniformLocation(shaderProgram.id(), "ambient");
		uDiffuse = glGetUniformLocation(shaderProgram.id(), "diffuse");
		uSpecular = glGetUniformLocation(shaderProgram.id(), "specular");
		uShininess = glGetUniformLocation(shaderProgram.id(), "shininess");

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		msglVersion();
		return !msglError();
	}

	bool end() {
		windowShouldClose();
		return true;
	}

	void activateUniforms(glm::vec4& _light0, glm::vec4& _light1, Material* m) {
		glUniformMatrix4fv(uModelViewMatrix, 1, false, glm::value_ptr(modelViewMatrix));
		glUniformMatrix4fv(uProjectionMatrix, 1, false, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(uNormalMatrix, 1, false, glm::value_ptr(normalMatrix));
		glUniform4fv(uLight0_position, 1, glm::value_ptr(_light0));
		glUniform4fv(uLight0_color, 1, glm::value_ptr(light0.color()));
		glUniform4fv(uLight1_position, 1, glm::value_ptr(_light1));
		glUniform4fv(uLight1_color, 1, glm::value_ptr(light1.color()));
		glUniform4fv(uAmbient, 1, glm::value_ptr(m->ambient));
		glUniform4fv(uDiffuse, 1, glm::value_ptr(m->diffuse));
		glUniform4fv(uSpecular, 1, glm::value_ptr(m->specular));
		glUniform1f(uShininess, m->shininess);
	}

	/*This function is called "shoemaker" because it is the name of the method
	we are using to implement a trackball
	Note that this fuction assumes that r = 1, so the math is simplfied becasue of that*/
	glm::vec3 shoemaker(int x, int y) {
		glm::vec3 coordinates(0.0);
		std::tuple<int, int> w = windowSize();
		double width = std::get<0>(w);
		double height = std::get<1>(w);

		/*First off, we need the xy coords to be in device coords in the range: [-1, 1]
		Y coordinate needs to be negated due to screen coordinates origin starting at the top left corner
		The sign of y is positive going down*/
		coordinates.x = (2 * x - width) / width;
		coordinates.y = -(2 * y - height) / height;
		coordinates.x = glm::clamp(coordinates.x, -1.0f, 1.0f);
		coordinates.y = glm::clamp(coordinates.y, -1.0f, 1.0f);

		float xySquared = coordinates.x*coordinates.x + coordinates.y*coordinates.y;
		/*Since the radius = 1, the following condition is eqivalent to:
		if (sqrt(xySquared) <= powf(radius, 2)).
		This condition basically checks to see if I am in the sphere
		so it doesn't matter if xySquared is square rooted*/
		if (xySquared <= 1.0) {
			coordinates.z = sqrt(1.0 - xySquared);
		}
		else {
			/*Since r = 1, this is also equivalent to:
			(r / (sqrt(xySquared))) * (x, y, 0), so just normalizing the vector will do;
			Which brings the point outside the sphere back to a closest point on the sphere*/
			coordinates = glm::normalize(coordinates);
		}
		return coordinates;
	}

	void debug() {
		std::cerr << "Normal Prev: " << glm::to_string(prevCoord) << std::endl;
		std::cerr << "Normal Current: " << glm::to_string(currentCoord) << std::endl;
		std::cerr << "angle: " << angle << std::endl;
		std::cerr << "axis: " << glm::to_string(axis) << std::endl;
		std::cerr << "Quaternion Rotation Matrix: " << glm::to_string(newRotation) << std::endl << std::endl;
	}

	bool render() {
		glm::vec4 _light0;
		glm::vec4 _light1;
		glm::mat4 lookAtMatrix;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		std::tuple<int, int> w = windowSize();
		double ratio = double(std::get<0>(w)) / double(std::get<1>(w));
		mainCamera.perspectiveMatrix(projectionMatrix, ratio);
		mainCamera.lookAtMatrix(lookAtMatrix);

		/*Set light & material properties for the teapot;
		lights are transformed by current modelview matrix
		such that they are positioned correctly in the scene.*/
		_light0 = lookAtMatrix * light0.position4();
		_light1 = lookAtMatrix * light1.position4();

		/*glm::mat4 model = glm::mat4();//Load Identity
		model *= rotation;
		model = glm::translate(model, teapot.position);
		modelViewMatrix = lookAtMatrix * model;*/

		modelViewMatrix = glm::translate(lookAtMatrix, teapot.position);
		//Uses the last rotation
		modelViewMatrix *= rotation;

		normalMatrix = glm::inverseTranspose(modelViewMatrix);
		shaderProgram.activate();
		activateUniforms(_light0, _light1, teapot.material);
		teapot.draw();

		int mbFlags = mouseButtonFlags();
		std::tuple<int, int> mousePosition = mouseCurrentPosition();
		std::tuple<int, int> prevMousePosition = mousePreviousPosition();

		if (mbFlags == MOUSE_BUTTON_LEFT) {
			std::cerr << "Left mouse button is down" << std::endl;
			std::cerr << "Current mouse position: " << std::get<0>(mousePosition) << ", " <<
				std::get<1>(mousePosition) << std::endl;
			std::cerr << "Previous mouse position: " << std::get<0>(prevMousePosition) << ", " <<
				std::get<1>(prevMousePosition) << std::endl;

			if (!held) {
				//Upon the first press, get the normalized previous coordinates
				prevCoord = shoemaker(std::get<0>(prevMousePosition), std::get<1>(prevMousePosition));
				held = true;
			}
			else if (held) {
				//If the button is held, continuously get the normalized current coordinates
				currentCoord = shoemaker(std::get<0>(mousePosition), std::get<1>(mousePosition));

				//Since r = 1, you can just simply take the dot product
				angle = glm::dot(prevCoord, currentCoord);
				//angle = glm::clamp(angle, 0.0f, 0.5f);

				//Calculate the axis in the camera's coordinate system
				axis = glm::cross(prevCoord, currentCoord);
				/*glm::mat3 cam2object = glm::inverse(glm::mat3(lookAtMatrix));//Or is it modelview?
				axis = cam2object * axis;*/

				//This prevents the teapot from "resetting" then the button is pressed and held in one place
				if (glm::dot(prevCoord, currentCoord) < 0.9f) {
					//Set your old rotation to the prior new one
					rotation = newRotation;

					//Update the Model-View with another rotate for the next iteration
					Quaternion rotationQuat(axis, angle);
					newRotation = rotationQuat.getRotationMatrix();
					//Apply the new rotation to the old one
					rotation *= newRotation;
				}
			}

			/*Quaternion a(glm::vec3(0, 1, 0), glm::radians(90.0), "A");
			a.debug();
			Quaternion b(glm::vec3(1, 0, 0), glm::radians(45.0), "B");
			b.debug();
			Quaternion result;
			result = a.multiply(b);
			result.debug();*/
			debug();

		}
		else if (mbFlags == MOUSE_BUTTON_RIGHT) {
			//Same implementation as above, except I am using Euler's angles for comparison
			std::cerr << "Right mouse button is down" << std::endl;
			std::cerr << "Current mouse position: " << std::get<0>(mousePosition) << ", " <<
				std::get<1>(mousePosition) << std::endl;
			std::cerr << "Previous mouse position: " << std::get<0>(prevMousePosition) << ", " <<
				std::get<1>(prevMousePosition) << std::endl;
			if (!held) {
				prevCoord = shoemaker(std::get<0>(prevMousePosition), std::get<1>(prevMousePosition));
				held = true;
			}
			else if (held) {
				currentCoord = shoemaker(std::get<0>(mousePosition), std::get<1>(mousePosition));
				angle = glm::dot(prevCoord, currentCoord);
				axis = glm::cross(prevCoord, currentCoord);
				rotation = newRotation;
				newRotation = glm::rotate(glm::degrees(angle), axis);
				rotation *= newRotation;
			}
		}
		else {
			//If no button is held, set this to false
			held = false;
		}

		if (isKeyPressed('Q')) {
			end();
		}
		else if (isKeyPressed(GLFW_KEY_EQUAL)) {
		}
		else if (isKeyPressed(GLFW_KEY_MINUS)) {
		}
		else if (isKeyPressed('R')) {
			initCamera();
			initRotationDelta();
			initLights();
			initRotation();
			printf("Eye position, up vector, rotation matrix, and rotation delta reset.\n");
		}
		else if (isKeyPressed(GLFW_KEY_LEFT)) {
			mainCamera.rotateCameraRight();
		}
		else if (isKeyPressed(GLFW_KEY_RIGHT)) {
			mainCamera.rotateCameraLeft();
		}
		else if (isKeyPressed(GLFW_KEY_UP)) {
			mainCamera.rotateCameraDown();
		}
		else if (isKeyPressed(GLFW_KEY_DOWN)) {
			mainCamera.rotateCameraUp();
		}
		else if (isKeyPressed('W')) {
			light0.rotateUp();
		}
		else if (isKeyPressed('S')) {
			light0.rotateUp();
		}
		else if (isKeyPressed('A')) {
			light0.rotateLeft();
		}
		else if (isKeyPressed('D')) {
			light0.rotateLeft();
		}
		else if (isKeyPressed('X')) {
			light0.roll();
		}
		else if (isKeyPressed('Y')) {
			light1.rotateUp();
		}
		else if (isKeyPressed('H')) {
			light1.rotateUp();
		}
		else if (isKeyPressed('G')) {
			light1.rotateLeft();
		}
		else if (isKeyPressed('J')) {
			light1.rotateLeft();
		}
		else if (isKeyPressed('N')) {
			light1.roll();
		}
		else if (isKeyPressed('O')) {
			mainCamera.forward();
		}
		else if (isKeyPressed('L')) {
			mainCamera.backward();
		}
		else if (isKeyPressed('K')) {
			mainCamera.panLeft();
		}
		else if (isKeyPressed(';')) {
			mainCamera.panRight();
		}
		else if (isKeyPressed('1')) {
			light0.toggle();
		}
		else if (isKeyPressed('2')) {
			light1.toggle();
		}
		return !msglError();
	}
};

int main(int argc, char* argv[]) {
	TeapotTrackballApp app(argc, argv);
	return app();
}