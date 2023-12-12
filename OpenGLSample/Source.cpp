#include <iostream>
#include <cstdlib>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using namespace std;

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

namespace {
    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.5f, 3.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

    float rotationAngle = 0.0f;
    float cameraSpeed = 0.05f;
    float yaw = -90.0f;
    float pitch = 0.0f;

    const char* const WINDOW_TITLE = "Final Project - Swimming Pool Courtyard";
    const int WINDOW_WIDTH = 1800;
    const int WINDOW_HEIGHT = 1600;

    struct GLMesh {
        GLuint vao;
        GLuint vbos[2];
        GLuint nIndices;
    };

    GLFWwindow* gWindow = nullptr;
    GLMesh gPool;
    GLMesh gWalkway;
    GLMesh gTable1, gTable2;
    GLuint gProgramId;
    GLuint textureID;  // Global variable for brick texture ID
    GLuint rippleTextureID; // Global variable for ripple texture ID
}

bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UCreatePool(GLMesh& mesh);
void UCreateWalkway(GLMesh& mesh);
void UCreateCube(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool isPerspective = true;

GLuint loadTexture(const char* texImagePath);


// VERTEX SHADER
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoords;

out vec2 TexCoords;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = position;
    gl_Position = projection * view * model * vec4(position, 1.0f);
    TexCoords = texCoords;
}
);


// FRAGMENT SHADER
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 TexCoords;
in vec3 FragPos;
out vec4 fragmentColor;

struct Light {
    vec3 position;
    vec3 color;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;
};

struct Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;
};

uniform Light light;
uniform Spotlight spotlight;
uniform vec3 viewPos;
uniform sampler2D ourTexture;
uniform sampler2D rippleTexture;
uniform bool isPool;

void main() {
    vec3 norm;
    if (isPool) {
        norm = vec3(0.0, 1.0, 0.0); // Upward normal for flat pool surface
    }
    else {
        norm = vec3(0.0, 1.0, 0.0); // Adjust normal
    }

    // Diffuse lighting
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuseStrength * diff * light.color;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    vec3 specular = light.specularStrength * spec * light.color;

    vec3 ambient = light.ambientStrength * light.color;
    vec3 result = (ambient + diffuse + specular);

    // Spotlight calculations
    vec3 spotlightDir = normalize(spotlight.position - FragPos);
    float theta = dot(spotlightDir, normalize(-spotlight.direction));
    float epsilon = spotlight.cutOff - spotlight.outerCutOff;
    float intensity = clamp((theta - spotlight.outerCutOff) / epsilon, 0.0, 1.0);
    vec3 spotlightEffect = intensity * (spotlight.ambientStrength * ambient + spotlight.diffuseStrength * diffuse + spotlight.specularStrength * specular) * spotlight.color;
    result += spotlightEffect * intensity;

    if (isPool) {
        vec4 poolColor = vec4(0.0, 0.4, 0.7, 1.0);
        vec4 rippleColor = texture(rippleTexture, TexCoords);
        fragmentColor = vec4(result, 1.0) * mix(poolColor, rippleColor, 0.5);
    }
    else {
        fragmentColor = vec4(result, 1.0) * texture(ourTexture, TexCoords);
    }
}
);


int main(int argc, char* argv[]) {
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    textureID = loadTexture("brick.jpg");
    if (textureID == 0) {
        cout << "Failed to load the texture" << endl;
        return EXIT_FAILURE;
    }

    // Load the additional ripple texture
    rippleTextureID = loadTexture("water_ripple.jpg");
    if (rippleTextureID == 0) {
        cout << "Failed to load the ripple texture" << endl;
        return EXIT_FAILURE;
    }

    UCreatePool(gPool);
    UCreateWalkway(gWalkway);
    UCreateCube(gTable1);
    UCreateCube(gTable2);

    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(gWindow)) {
        UProcessInput(gWindow);
        URender();
        glfwPollEvents();
    }

    UDestroyMesh(gPool);
    UDestroyMesh(gWalkway);
    UDestroyShaderProgram(gProgramId);
    UDestroyMesh(gTable1);
    UDestroyMesh(gTable2);


    exit(EXIT_SUCCESS);
}

GLuint loadTexture(const char* texImagePath) {
    GLuint textureId;
    int imgWidth, imgHeight, imgChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img = stbi_load(texImagePath, &imgWidth, &imgHeight, &imgChannels, 0);
    if (img) {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set the texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(img);
        std::cout << "Texture loaded successfully from " << texImagePath << std::endl;
    }
    else {
        std::cout << "Texture failed to load at path: " << texImagePath << std::endl;
        stbi_image_free(img);
    }

    std::cout << "Successfully loaded image for texture" << std::endl;

    return textureId;
}



// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    glfwSetCursorPosCallback(*window, mouse_callback);
    glfwSetScrollCallback(*window, scroll_callback);
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPosition += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPosition -= cameraSpeed * cameraUp;

    // Camera Rotation
    float rotationSpeed = 1.0f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw += rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw -= rotationSpeed;

    // Update camera direction based on updated yaw
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    // Toggle projection mode when 'P' is pressed
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        isPerspective = !isPerspective;
    }
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400.0f, lastY = 300.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    float sensitivity = 0.002f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraSpeed += static_cast<float>(yoffset) * 0.01f;
    if (cameraSpeed < 0.01f)
        cameraSpeed = 0.01f;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void URender() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model;
    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    // glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 200.0f);
    glm::mat4 projection;
    if (isPerspective) {
        projection = glm::perspective(glm::radians(55.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 200.0f);
    }
    else {
        // Set the orthographic projection parameters
        float orthoWidth = 5.0f;  // Smaller value for more zoom
        float orthoHeight = 5.0f; // Smaller value for more zoom

        projection = glm::ortho(-orthoWidth / 2, orthoWidth / 2, -orthoHeight / 2, orthoHeight / 2, 0.1f, 200.0f);
    }

    // Use the projection matrix in shader
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(gProgramId);

    // Set up the light propertiesd
    glm::vec3 lightPos(2.0f, 6.0f, 3.0f); // Position of the light
    glm::vec3 lightColor(1.0f, 0.95f, 0.8f); // Color of the light
    glUniform3f(glGetUniformLocation(gProgramId, "light.position"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(gProgramId, "light.color"), lightColor.r, lightColor.g, lightColor.b);
    glUniform1f(glGetUniformLocation(gProgramId, "light.ambientStrength"), 0.3f);
    glUniform1f(glGetUniformLocation(gProgramId, "light.diffuseStrength"), 1.0f);
    glUniform3f(glGetUniformLocation(gProgramId, "viewPos"), cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Set up the spotlight properties
    glm::vec3 spotlightPos = cameraPosition - glm::vec3(0.0f, 0.0f, 0.5f); // Adjust the offset as needed
    glm::vec3 spotlightDir = glm::normalize(cameraFront); // Direction the camera is facing
    glm::vec3 spotlightColor = glm::vec3(1.0f, 0.9f, 0.6f); // Warm sunlight color
    float spotlightCutOff = glm::cos(glm::radians(25.5f));
    float spotlightOuterCutOff = glm::cos(glm::radians(25.5f));

    glUniform3f(glGetUniformLocation(gProgramId, "spotlight.position"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
    glUniform3f(glGetUniformLocation(gProgramId, "spotlight.direction"), spotlightDir.x, spotlightDir.y, spotlightDir.z);
    glUniform3f(glGetUniformLocation(gProgramId, "spotlight.color"), spotlightColor.r, spotlightColor.g, spotlightColor.b);
    glUniform1f(glGetUniformLocation(gProgramId, "spotlight.cutOff"), spotlightCutOff);
    glUniform1f(glGetUniformLocation(gProgramId, "spotlight.outerCutOff"), spotlightOuterCutOff);
    glUniform1f(glGetUniformLocation(gProgramId, "spotlight.ambientStrength"), 0.05f);
    glUniform1f(glGetUniformLocation(gProgramId, "spotlight.diffuseStrength"), 0.15f);

    // Set the shader uniforms
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    // GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the brick texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(gProgramId, "ourTexture"), 0);

    // Render first table
    model = glm::translate(glm::vec3(1.4f, -0.4f, 0.0f)); // Adjust position for first table
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // Scale the table
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(gTable1.vao);
    glDrawArrays(GL_TRIANGLES, 0, gTable1.nIndices);

    // Render second table
    model = glm::translate(glm::vec3(1.4f, -0.4f, 0.7f)); // Adjust position for second table
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // Scale the table
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(gTable2.vao);
    glDrawArrays(GL_TRIANGLES, 0, gTable2.nIndices);

    // Render third table
    model = glm::translate(glm::vec3(1.4f, -0.4f, -0.5f)); // Adjust position for second table
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // Scale the table
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(gTable2.vao);
    glDrawArrays(GL_TRIANGLES, 0, gTable2.nIndices);

    // Render fourth table (mirror position of the first table)
    model = glm::translate(glm::vec3(-1.4f, -0.4f, 0.0f)); // Adjust position for fourth table
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // Scale the table
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(gTable1.vao); // Assuming the same mesh for all tables
    glDrawArrays(GL_TRIANGLES, 0, gTable1.nIndices);

    // Render fifth table (mirror position of the second table)
    model = glm::translate(glm::vec3(-1.4f, -0.4f, 0.7f)); // Adjust position for fifth table
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // Scale the table
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(gTable2.vao); // Assuming the same mesh for all tables
    glDrawArrays(GL_TRIANGLES, 0, gTable2.nIndices);

    // Render sixth table (mirror position of the third table)
    model = glm::translate(glm::vec3(-1.4f, -0.4f, -0.5f)); // Adjust position for sixth table
    model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // Scale the table
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(gTable2.vao); // Assuming the same mesh for all tables
    glDrawArrays(GL_TRIANGLES, 0, gTable2.nIndices);


    // Activate the ripple texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rippleTextureID);
    glUniform1i(glGetUniformLocation(gProgramId, "rippleTexture"), 1);

    // Render Pool
    model = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(gProgramId, "isPool"), GL_TRUE); // Indicate pool rendering
    glBindVertexArray(gPool.vao);
    glDrawElements(GL_TRIANGLES, gPool.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    // Render Walkway
    model = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform1i(glGetUniformLocation(gProgramId, "isPool"), GL_FALSE); // Indicate walkway rendering
    glBindVertexArray(gWalkway.vao);
    glDrawElements(GL_TRIANGLES, gWalkway.nIndices, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    glfwSwapBuffers(gWindow);
}

// Create Tables
void UCreateCube(GLMesh& mesh) {
    // Vertices for a cube
    GLfloat vertices[] = {
        // Positions           // Texture Coords
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f
    };

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    mesh.nIndices = sizeof(vertices) / (5 * sizeof(GLfloat)); // Number of vertices
}


// Function to create a 3D pool
void UCreatePool(GLMesh& mesh) {
    GLfloat verts[] = {
        // Positions          // Texture Coords
        // Top face
        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f,
         1.0f, 0.0f, -1.0f,   1.0f, 0.0f,
         1.0f, 0.0f,  1.0f,   1.0f, 1.0f,
        -1.0f, 0.0f,  1.0f,   0.0f, 1.0f,

        // Bottom face
        -1.0f, -0.6f, -1.0f,  0.0f, 0.0f,
         1.0f, -0.6f, -1.0f,  1.0f, 0.0f,
         1.0f, -0.6f,  1.0f,  1.0f, 1.0f,
        -1.0f, -0.6f,  1.0f,  0.0f, 1.0f,
    };

    GLushort indices[] = {
        // Top face
        0, 1, 2,
        2, 3, 0,

        // Bottom face
        4, 5, 6,
        6, 7, 4,

        // Side faces
        0, 1, 5,
        5, 4, 0,

        1, 2, 6,
        6, 5, 1,

        2, 3, 7,
        7, 6, 2,

        3, 0, 4,
        4, 7, 3
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerTexture = 2;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerTexture);

    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

// Function to create a walkway around the pool
void UCreateWalkway(GLMesh& mesh) {
    GLfloat verts[] = {
        // Positions          // Texture Coords
        // Top face (modified texture coords for 3x tiling)
        -1.2f, 0.0f, -1.2f,   0.0f, 0.0f,
         1.2f, 0.0f, -1.2f,   3.0f, 0.0f,  // x3 tiling horizontally
         1.2f, 0.0f,  1.2f,   3.0f, 3.0f,  // x3 tiling both horizontally and vertically
        -1.2f, 0.0f,  1.2f,   0.0f, 3.0f,  // x3 tiling vertically

        -1.0f, 0.0f, -1.0f,   0.0f, 0.0f,
         1.0f, 0.0f, -1.0f,   1.0f, 0.0f,
         1.0f, 0.0f,  1.0f,   1.0f, 1.0f,
        -1.0f, 0.0f,  1.0f,   0.0f, 1.0f,
    };

    GLushort indices[] = {
        0, 1, 4,
        1, 5, 4,

        1, 2, 5,
        2, 6, 5,

        2, 3, 6,
        3, 7, 6,

        3, 0, 7,
        0, 4, 7
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerTexture = 2;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerTexture);

    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Link and use the program
    glLinkProgram(programId);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader

    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    // Initialize the uniform
    glUniform1i(glGetUniformLocation(programId, "isPool"), GL_FALSE);

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}