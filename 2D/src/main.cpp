// This example is heavily based on the tutorial at https://open.gl

////////////////////////////////////////////////////////////////////////////////
// OpenGL Helpers to reduce the clutter
#include "helpers.h"
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
// Linear Algebra Library
#include <Eigen/Dense>
// Timer
#include <chrono>
// I/O
#include <iostream>
// Include lists
#include <list>
// Algorithm for finding element in a list
#include <algorithm>
// Trig functions for rotation
#include <math.h>
////////////////////////////////////////////////////////////////////////////////

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
Eigen::MatrixXf V(2,3);

// Contains the per-vertex color
Eigen::MatrixXf C(3,3);

// vars
bool test = false;
bool test2 = false;
bool insert_mode = false;
int triangles = 0;
int clicks = 0;
double xnew;
double ynew;
            
bool trans_mode = false;  // `o`: Triangle translation mode // Little Bug with Trans mode..
bool mouse_down = false; 
bool inside_tri = false;  // true if cursor is inside triangle
int current_tri = -100;
bool trans_flag = false;

double x_prev = 100;   // init to impossible values
double y_prev = 100;

bool delete_mode = false;
std::list <int> deleted;    // list of deleted triangles 

bool color_mode = false;
int color_index;
float color_buffer[3][3];
float color_1;
            
            
// Helper function for color mode
void color(int num) {
    
    switch(num) {
        case (1):
            C.col(color_index) << 1,1,0;
            break;
        case (2):
            C.col(color_index) << 0,1,1;
            break;
        case (3):
            C.col(color_index) << 1,1,0;
            break;
        case (4):
            C.col(color_index) << 1,1,1;
            break;
        case (5):
            C.col(color_index) << 0,0,0;
            break;
        case (6):
            C.col(color_index) << 1,0,1;
            break;
        case (7):
            C.col(color_index) << 0.5,0,0;
            break;
        case (8):
            C.col(color_index) << 0,0,1;
            break;
        case (9):
            C.col(color_index) << 0,1,0;
            break;
        default:
            break;
    }

    VBO_C.update(C);
}

// Scaling function for translation mode
void scale(double amt) {
    
    // Calculate barycenter
    double x1 = V.col(current_tri)(0);
    double x2 = V.col(current_tri+1)(0);
    double x3 = V.col(current_tri+2)(0);

    double y1 = V.col(current_tri)(1);
    double y2 = V.col(current_tri+1)(1);
    double y3 = V.col(current_tri+2)(1);

    double barycent_x = (x1 + x2 + x3) / 3; 
    double barycent_y = (y1 + y2 + y3) / 3; 

    // Calculate scaled vertex positions
    double x1s, x2s, x3s, y1s, y2s, y3s;

    x1s = x1 * amt;
    x2s = x2 * amt;
    x3s = x3 * amt;
    y1s = y1 * amt;
    y2s = y2 * amt;
    y3s = y3 * amt;

    // Adjust values to fit the original barycenter
    double barycent_x_new = (x1s + x2s + x3s) / 3; 
    double barycent_y_new = (y1s + y2s + y3s) / 3; 
    
    double adjust_x = barycent_x - barycent_x_new;
    double adjust_y = barycent_y - barycent_y_new;

    x1s += adjust_x;
    x2s += adjust_x;
    x3s += adjust_x;
    y1s += adjust_y;
    y2s += adjust_y;
    y3s += adjust_y;

    // Apply scaled vertex positions
    V.col(current_tri) << x1s, y1s;
    V.col(current_tri+1) << x2s, y2s;
    V.col(current_tri+2) << x3s, y3s;

}

// Rotation function for translation mode
void rotate( double dir ) {

    // Convert degrees to radians:
    if (dir == 10)
        dir = 0.174533;
    else
        dir = -0.174533;

    // Calculate barycenter
    double x1 = V.col(current_tri)(0);
    double x2 = V.col(current_tri+1)(0);
    double x3 = V.col(current_tri+2)(0);

    double y1 = V.col(current_tri)(1);
    double y2 = V.col(current_tri+1)(1);
    double y3 = V.col(current_tri+2)(1);

    double barycent_x = (x1 + x2 + x3) / 3; 
    double barycent_y = (y1 + y2 + y3) / 3; 

    // Calculate rotated vertex positions
    double x1rot, x2rot, x3rot, y1rot, y2rot, y3rot;

    x1rot = ((x1 - barycent_x) * cos(dir) - (y1 - barycent_y) * sin(dir) + barycent_x);
    y1rot = ((x1 - barycent_x) * sin(dir) + (y1 - barycent_y) * cos(dir) + barycent_y);
    x2rot = ((x2 - barycent_x) * cos(dir) - (y2 - barycent_y) * sin(dir) + barycent_x);
    y2rot = ((x2 - barycent_x) * sin(dir) + (y2 - barycent_y) * cos(dir) + barycent_y);
    x3rot = ((x3 - barycent_x) * cos(dir) - (y3 - barycent_y) * sin(dir) + barycent_x);
    y3rot = ((x3 - barycent_x) * sin(dir) + (y3 - barycent_y) * cos(dir) + barycent_y);
    
    // Apply rotated vertex positions
    V.col(current_tri) << x1rot, y1rot;
    V.col(current_tri+1) << x2rot, y2rot;
    V.col(current_tri+2) << x3rot, y3rot;

}

// callback function for mouse movement
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // cout << "X" << xpos << "Y" << ypos << endl;
	
	// Get viewport size (canvas in number of pixels) 
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Get the size of the window (may be different than the canvas size on retina displays)
	int width_window, height_window;
	glfwGetWindowSize(window, &width_window, &height_window);

	// Deduce position of the mouse in the viewport
	double highdpi = (double) width / (double) width_window;// - could be that resolution of pixels
                                                            //   does not == # real pixels. so always need to rescale
	xpos *= highdpi;
	ypos *= highdpi;

	// Convert screen position to world coordinates
	double xworld = ((xpos/double(width))*2)-1; // - keeps range in -1,1 for x, then same for y below, but ...
	double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    // `i` Triangle insertion mode
    if (insert_mode) {
        switch (clicks) {
            case 0:
                V.col(triangles*3) << xworld, yworld;
                V.col(triangles*3+1) << xworld, yworld;
                V.col(triangles*3+2) << xworld, yworld;
                break;
            case 1:
                V.col(triangles*3+1) << xworld, yworld;
                V.col(triangles*3+2) << xworld, yworld;
                break;
            case 2:
                V.col(triangles*3+2) << xworld, yworld;
                break;
        }
    }
    
    if (trans_mode && inside_tri && clicks == 1 && mouse_down) {
        double x_dif;
        double y_dif;
        if (x_prev > 5 && y_prev > 5) {
            x_prev = xworld;
            y_prev = yworld;
        }
        else { // transform triangle by change in mouse pos
            x_dif = xworld - x_prev;
            y_dif = yworld - y_prev;
            for (int i = current_tri; i <= current_tri+2; i++) {
                V.col(i) << (((double) V.col(i)(0)) + x_dif), (((double) V.col(i)(1)) + y_dif);
            }
            x_prev = xworld;
            y_prev = yworld;
        }
    }

    // Upload the change to the GPU
	VBO.update(V);
}

// callback function for mouse press
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	// Get viewport size (canvas in number of pixels)
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// Get the size of the window (may be different than the canvas size on retina displays)
	int width_window, height_window;
	glfwGetWindowSize(window, &width_window, &height_window);

	// Get the position of the mouse in the window
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Deduce position of the mouse in the viewport
	double highdpi = (double) width / (double) width_window;
	xpos *= highdpi;
	ypos *= highdpi;

	// Convert screen position to world coordinates
	double xworld = ((xpos/double(width))*2)-1;
	double yworld = (((height-1-ypos)/double(height))*2)-1; // NOTE: y axis is flipped in glfw

    // Track if mouse is clicked and held
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_down = true;
    }
    else {
        mouse_down = false;
    }

	// Draw edges for insertion mode
    if (insert_mode && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        xnew = xworld;
        ynew = yworld;
        clicks++;

        // Resize position matrix
        V.conservativeResize(V.rows(), triangles * 3 + clicks + 3); // added 3 cols for new tri buffer
        V.col(triangles*3+clicks) << xworld, yworld;
        V.col(triangles*3+clicks+1) << xworld, yworld;
        V.col(triangles*3+clicks+2) << xworld, yworld;

        // Resize the color matrix
        C.conservativeResize(C.rows(),triangles * 3 + clicks + 3);
        for (int i=triangles*3; i < triangles * 3 + 3; i ++) {
            C.col(i) << 1, 0, 0; 
        }

        V.col(triangles * 3 + clicks - 1) << xnew, ynew;
        if (clicks == 3) {
            triangles++;
            std::cout << " num triangles:: " << triangles << std::endl;
            insert_mode = false;
            clicks = 0;
        }
    }

    trans_flag = false; // to track if we should end trans func

    // Detect if cursor is inside triangle (only need to do this extra computation if in translate mode)
    if ((trans_mode || delete_mode ) && action == GLFW_PRESS) {
        for (int i = 0; i < triangles * 3; i +=3 ) { 

            double p0x = V.col(i)(0);
            double p0y = V.col(i)(1);
            double p1x = V.col(i+1)(0);
            double p1y = V.col(i+1)(1);
            double p2x = V.col(i+2)(0);
            double p2y = V.col(i+2)(1);

            double px = xworld;
            double py = yworld;

            float area = 0.5 *(-p1y*p2x + p0y*(-p1x + p2x) + p0x*(p1y - p2y) + p1x*p2y);
            float s = 1.0/(2.0*area)*(p0y*p2x - p0x*p2y + (p2y - p0y)*px + (p0x - p2x)*py);
            float t = 1.0/(2.0*area)*(p0x*p1y - p0y*p1x + (p0y - p1y)*px + (p1x - p0x)*py);

            // evaluate s, t and 1-s-t. The point p is inside the triangle if and only if they are all positive.
            if (s > 0 && t > 0 && (1 - s - t) > 0) {
                if (trans_mode) {
                    trans_flag = true;
                    inside_tri = true;
                    current_tri = i;

                    // Set active triangle to blue
                    for (int j=current_tri; j < current_tri + 3; j++) {
                        color_buffer[j-current_tri][0] = C.col(j)(0);
                        color_buffer[j-current_tri][1] = C.col(j)(1);
                        color_buffer[j-current_tri][2] = C.col(j)(2);
                        C.col(j) << 0, 0, 1; 
                    }

                    clicks = 1;
                } 
                else if (delete_mode) {
                    deleted.push_front((i/3) + 1);  
                    delete_mode = false;
                    std::cout << "deleting " << (i/3) + 1 << " ... " << std::endl;
                }
            }
        }

        // END TRANS MODE - set non active triangle to red :
        if (trans_mode && trans_flag == false ) {
            std::cout << "Exiting Trans Mode . " << std::endl;
            trans_mode = false;
            for (int j=current_tri; j < current_tri + 3; j++) 
                C.col(j) << color_buffer[j-current_tri][0], color_buffer[j-current_tri][1], color_buffer[j-current_tri][2]; 
            current_tri = -100;
            x_prev = 100;
            y_prev = 100;
        }
    }

    // if color mode, find closest vertex
    if (color_mode) {
        double min_dist = 100;
        double new_dist;

        for (int i=0; i<V.cols(); i++) {
            new_dist = sqrt(pow(xworld - V.col(i)(0), 2) + pow(yworld - V.col(i)(1), 2));
            if (new_dist < min_dist) {
                min_dist = new_dist;
                color_index = i;
            }
        }
    }

	// Upload the change to the GPU
    VBO_C.update(C);
	VBO.update(V);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// Update the position of the first vertex if the keys 1,2, or 3 are pressed
	switch (key) {
		case GLFW_KEY_1:
            if (color_mode)
                color(1);
			break;
		case GLFW_KEY_2:
            if (color_mode)
                color(2);
			break;
		case GLFW_KEY_3:
            if (color_mode)
                color(3);
			break;
		case GLFW_KEY_4:
            if (color_mode)
                color(4);
			break;
		case GLFW_KEY_5:
            if (color_mode)
                color(5);
			break;
		case GLFW_KEY_6:
            if (color_mode)
                color(6);
			break;
		case GLFW_KEY_7:
            if (color_mode)
                color(7);
			break;
		case GLFW_KEY_8:
            if (color_mode)
                color(8);
			break;
		case GLFW_KEY_9:
            if (color_mode)
                color(9);
			break;
        case GLFW_KEY_I:
            if (!insert_mode) {
                insert_mode = true;
                std::cout << "entering insert mode ... " << std::endl;
                clicks = 0;
            }
			break;
        case GLFW_KEY_O:
			if (!trans_mode){
                clicks = 0;
                trans_mode = true;
                std::cout << "entering translate mode ... " << std::endl;
            }
			break;
        case GLFW_KEY_H: // Rotate 10 degrees clockwise
			if (trans_mode && action == GLFW_PRESS){
                rotate(10);
            } 
			break;
        case GLFW_KEY_J: // Rotate 10 degrees counter-clockwise
			if (trans_mode && action == GLFW_PRESS){
                rotate(-10);
            } 
			break;
        case GLFW_KEY_K: // Scale up by 25%
			if (trans_mode && action == GLFW_PRESS){
                scale(1.25);
            }
			break;
        case GLFW_KEY_L: // Scale down by 25%
			if (trans_mode && action == GLFW_PRESS){
                scale(0.75);
            }
			break;
        case GLFW_KEY_D:
            if (!delete_mode){
                delete_mode = true;
                std::cout << "entering delete mode ... " << std::endl;
            }
            break;
        case GLFW_KEY_C:
            if (!color_mode){
                color_mode = true;
                std::cout << "entering color mode ... " << std::endl;
            }
            break;
		default:
			break;
	}

	// Upload the change to the GPU
    VBO_C.update(C);
	VBO.update(V);
}

int main(void) {
	// Initialize the GLFW library
	if (!glfwInit()) {
		return -1;
	}

	// Activate supersampling
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Ensure that we get at least a 3.2 context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create a windowed mode window and its OpenGL context
	GLFWwindow * window = glfwCreateWindow(640, 480, "[Float] Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	// Load OpenGL and its extensions
	if (!gladLoadGL()) {
		printf("Failed to load OpenGL and its extensions");
		return(-1);
	}
	printf("OpenGL Version %d.%d loaded", GLVersion.major, GLVersion.minor);

	int major, minor, rev;
	major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
	printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
	printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Initialize the VAO
	// A Vertex Array Object (or VAO) is an object that describes how the vertex
	// attributes are stored in a Vertex Buffer Object (or VBO). This means that
	// the VAO is not the actual object storing the vertex data,
	// but the descriptor of the vertex data.
	VertexArrayObject VAO;
	VAO.init();
	VAO.bind();

	// Initialize the VBO with the vertices data
	// A VBO is a data container that lives in the GPU memory
	VBO.init();

	V.resize(2,3);
    V << 0,0 , 0,0 , 0,0;
	VBO.update(V);

    // Second VBO for colors
    VBO_C.init();

    C.resize(3,3);
    C <<
    1,  1, 1,
    0,  0, 0,
    0,  0, 0;

    VBO_C.update(C);

	// Initialize the OpenGL Program
	// A program controls the OpenGL pipeline and it must contains
	// at least a vertex shader and a fragment shader to be valid
	Program program;
	const GLchar* vertex_shader = R"(
		#version 150 core
		in vec2 position;
        in vec3 color;
        out vec3 f_color;
		void main() {
			gl_Position = vec4(position, 0.0, 1.0);
            f_color = color;
		}
	)";

	const GLchar* fragment_shader = R"(
		#version 150 core
        in vec3 f_color;

		uniform vec3 triangleColor;
		out vec4 outColor;

		void main() {
		    outColor = vec4(f_color, 1.0);
		}
	)";

	// Compile the two shaders and upload the binary to the GPU
	// Note that we have to explicitly specify that the output "slot" called outColor
	// is the one that we want in the fragment buffer (and thus on screen)
	program.init(vertex_shader, fragment_shader, "outColor");
	program.bind();

	// The vertex shader wants the position of the vertices as an input.
	// The following line connects the VBO we defined above with the position "slot"
	// in the vertex shader
	program.bindVertexAttribArray("position", VBO);
    program.bindVertexAttribArray("color",VBO_C);

	// Save the current time --- it will be used to dynamically change the triangle color
	auto t_start = std::chrono::high_resolution_clock::now();

	// Register the keyboard callback
	glfwSetKeyCallback(window, key_callback);

	// Register the mouse press callback
	glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Register the cursor movement callback
    glfwSetCursorPosCallback(window, cursor_position_callback);

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {

		// Set the size of the viewport (canvas) to the size of the application window (framebuffer)
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Bind your VAO (not necessary if you have only one)
		VAO.bind();

		// Bind your program
		program.bind();

		// Set the uniform value depending on the time difference
		auto t_now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
		glUniform3f(program.uniform("triangleColor"),  2.0f, 0.0f, 0.0f);

		// Clear the framebuffer
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

        // Draw triangle area
        for (int i = 1; i <= triangles; i++) {
            if (std::find(deleted.begin(), deleted.end(), i) != deleted.end()) // Don't draw deleted triangles
                continue;
            glDrawArrays(GL_TRIANGLES, 3 * (i-1), 3);
            }

        glDrawArrays(GL_LINE_LOOP, (triangles*3), 3);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	// Deallocate opengl memory
	program.free();
	VAO.free();
	VBO.free();

	// Deallocate glfw internals
	glfwTerminate();
	return 0;
}

