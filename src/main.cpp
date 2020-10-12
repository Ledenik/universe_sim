#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <ctime>

#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"

#include "../include/shader.h"
#include "../include/camera.h"

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

#include "../include/body3d.h"
#include "../include/node3d.h"
#include "../include/universe.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

std::vector<glm::dvec3> preparePoints();
void print_tree(Node3D tree);

std::vector<Body3D> generateBodies(int num, glm::dvec3 offset);
std::vector<glm::vec3> prepare_positions(std::vector<Body3D> &bodies);
void populate_tree(Node3D &tree, std::vector<Body3D> &bodies);

glm::dvec3 random_point_elipsoid(glm::dvec3 dimensions);
std::vector<Body3D> generate_galaxy(int num_stars, bool black_hole, glm::dvec3 dimensions, glm::dvec3 center);

void sim(std::vector<Body3D> &bodies, double dt, glm::dvec3 center);

//settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
//camera
Camera camera(glm::vec3(0.0f, 0.0f, 2000.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
//timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main(int argc, char* argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Simulation", NULL, NULL);
    if(window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/vertex.shader", "shaders/fragment.shader");

    shader.use();
    srand((unsigned)time(0));

    Universe uni = Universe(3000, 1000.0f);
    uni.generate(glm::dvec3(1000.0f, 1000.0f, 250.0f));
    uni.setup();

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uni.simulate(deltaTime);
        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 20000.0f);
        shader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);

        uni.draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void print_tree(Node3D tree) {
    printf("Tree bounds - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.center.x - tree.length, tree.center.x + tree.length, tree.center.y - tree.length, tree.center.y + tree.length, tree.center.z - tree.length, tree.center.z + tree.length);
    printf("Tree center - {%f, %f}\n", tree.center.x, tree.center.y);
    printf("Tree body - {%f, %f}\n", tree.body->position.x, tree.body->position.y);
    printf("\n");

    if(tree.quads[0] != NULL) {
        printf("RTF - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[0]->center.x - tree.quads[0]->length, tree.quads[0]->center.x + tree.quads[0]->length, tree.quads[0]->center.y - tree.quads[0]->length, tree.quads[0]->center.y + tree.quads[0]->length, tree.quads[0]->center.z - tree.quads[0]->length, tree.quads[0]->center.z + tree.quads[0]->length);
        printf("RTF center - {%f, %f}\n", tree.quads[0]->center.x, tree.quads[0]->center.y);
        printf("RTF body - {%f, %f, %f}\n", tree.quads[0]->body->position.x, tree.quads[0]->body->position.y, tree.quads[0]->body->position.z);
        printf("\n");
    }
    if(tree.quads[1] != NULL) {
        printf("LTF - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[1]->center.x - tree.quads[1]->length, tree.quads[1]->center.x + tree.quads[1]->length, tree.quads[1]->center.y - tree.quads[1]->length, tree.quads[1]->center.y + tree.quads[1]->length, tree.quads[1]->center.z - tree.quads[1]->length, tree.quads[1]->center.z + tree.quads[1]->length);
        printf("LTF center - {%f, %f}\n", tree.quads[1]->center.x, tree.quads[1]->center.y);
        printf("LTF body - {%f, %f, %f}\n", tree.quads[1]->body->position.x, tree.quads[1]->body->position.y, tree.quads[1]->body->position.z);
        printf("\n");
    }
    if(tree.quads[2] != NULL) {
        printf("RBF - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[2]->center.x - tree.quads[2]->length, tree.quads[2]->center.x + tree.quads[2]->length, tree.quads[2]->center.y - tree.quads[2]->length, tree.quads[2]->center.y + tree.quads[2]->length, tree.quads[2]->center.z - tree.quads[2]->length, tree.quads[2]->center.z + tree.quads[2]->length);
        printf("RBF center - {%f, %f}\n", tree.quads[2]->center.x, tree.quads[2]->center.y);
        printf("RBF body - {%f, %f, %f}\n", tree.quads[2]->body->position.x, tree.quads[2]->body->position.y, tree.quads[2]->body->position.z);
        printf("\n");
    }
    if(tree.quads[3] != NULL) {
        printf("LBF - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[3]->center.x - tree.quads[3]->length, tree.quads[3]->center.x + tree.quads[3]->length, tree.quads[3]->center.y - tree.quads[3]->length, tree.quads[3]->center.y + tree.quads[3]->length, tree.quads[3]->center.z - tree.quads[3]->length, tree.quads[3]->center.z + tree.quads[3]->length);
        printf("LBF center - {%f, %f}\n", tree.quads[3]->center.x, tree.quads[3]->center.y);
        printf("LBF body - {%f, %f, %f}\n", tree.quads[3]->body->position.x, tree.quads[3]->body->position.y, tree.quads[3]->body->position.z);
        printf("\n");
    }
    if(tree.quads[4] != NULL) {
        printf("RTB - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[4]->center.x - tree.quads[4]->length, tree.quads[4]->center.x + tree.quads[4]->length, tree.quads[4]->center.y - tree.quads[4]->length, tree.quads[4]->center.y + tree.quads[4]->length, tree.quads[4]->center.z - tree.quads[4]->length, tree.quads[4]->center.z + tree.quads[4]->length);
        printf("RTB center - {%f, %f}\n", tree.quads[4]->center.x, tree.quads[4]->center.y);
        printf("RTB body - {%f, %f, %f}\n", tree.quads[4]->body->position.x, tree.quads[4]->body->position.y, tree.quads[4]->body->position.z);
        printf("\n");
    }
    if(tree.quads[5] != NULL) {
        printf("LTB - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[5]->center.x - tree.quads[5]->length, tree.quads[5]->center.x + tree.quads[5]->length, tree.quads[5]->center.y - tree.quads[5]->length, tree.quads[5]->center.y + tree.quads[5]->length, tree.quads[5]->center.z - tree.quads[5]->length, tree.quads[5]->center.z + tree.quads[5]->length);
        printf("LTB center - {%f, %f}\n", tree.quads[5]->center.x, tree.quads[5]->center.y);
        printf("LTB body - {%f, %f, %f}\n", tree.quads[5]->body->position.x, tree.quads[5]->body->position.y, tree.quads[5]->body->position.z);
        printf("\n");
    }
    if(tree.quads[6] != NULL) {
        printf("RBB - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[6]->center.x - tree.quads[6]->length, tree.quads[6]->center.x + tree.quads[6]->length, tree.quads[6]->center.y - tree.quads[6]->length, tree.quads[6]->center.y + tree.quads[6]->length, tree.quads[6]->center.z - tree.quads[6]->length, tree.quads[6]->center.z + tree.quads[6]->length);
        printf("RBB center - {%f, %f}\n", tree.quads[6]->center.x, tree.quads[6]->center.y);
        printf("RBB body - {%f, %f, %f}\n", tree.quads[6]->body->position.x, tree.quads[6]->body->position.y, tree.quads[6]->body->position.z);
        printf("\n");
    }
    if(tree.quads[7] != NULL) {
        printf("LBB - x{%f, %f}, y{%f, %f}, z{%f, %f}\n", tree.quads[7]->center.x - tree.quads[7]->length, tree.quads[7]->center.x + tree.quads[7]->length, tree.quads[7]->center.y - tree.quads[7]->length, tree.quads[7]->center.y + tree.quads[7]->length, tree.quads[7]->center.z - tree.quads[7]->length, tree.quads[7]->center.z + tree.quads[7]->length);
        printf("LBB center - {%f, %f}\n", tree.quads[7]->center.x, tree.quads[7]->center.y);
        printf("LBB body - {%f, %f, %f}\n", tree.quads[7]->body->position.x, tree.quads[7]->body->position.y, tree.quads[7]->body->position.z);
        printf("\n");
    }
}


//https://github.com/beltoforion/Barnes-Hut-Simulator

//https://github.com/chindesaurus/BarnesHut-N-Body

//https://github.com/rsnemmen/OpenCL-examples/tree/master/N-BodySimulation
