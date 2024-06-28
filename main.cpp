// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2023 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 5;
const static float kRadOrbitMoon = 2;
// Model transformation matrices
glm::mat4 g_sun, g_earth, g_moon, earthRotation, g_mercury;
float rotationSpeed = 2;



// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0;

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;
GLuint g_colorVbo =1;
GLuint g_earthTexID;
GLuint g_moonTexID;
GLuint g_mercuryTextID;



std::vector<float> g_vertexPositions;
std::vector<unsigned int> g_triangleIndices;
std::vector<float> g_vertexColors;

   

// camera model
class Camera {
public:
  inline float getFov() const { return m_fov; }
  inline void setFoV(const float f) { m_fov = f; }
  inline float getAspectRatio() const { return m_aspectRatio; }
  inline void setAspectRatio(const float a) { m_aspectRatio = a; }
  inline float getNear() const { return m_near; }
  inline void setNear(const float n) { m_near = n; }
  inline float getFar() const { return m_far; }
  inline void setFar(const float n) { m_far = n; }
  inline void setPosition(const glm::vec3 &p) { m_pos = p; }
  inline glm::vec3 getPosition() { return m_pos; }

  inline glm::mat4 computeViewMatrix() const {
    return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  }

  // Returns the projection matrix stemming from the camera intrinsic parameter.
  inline glm::mat4 computeProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
  }

private:
  glm::vec3 m_pos = glm::vec3(0, 0, 0);
  float m_fov = 45.f;        // Field of view, in degreesglEnableVertexAttribArray
  float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
  float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
  float m_far = 10.f; // Distance after which the geometry is excluded from the rasterization process


  public:
  void zoomIn(){
    this->m_fov-=2.5f; 
  }
  void zoomOut(){
    this->m_fov+=2.5f;
  }
};


Camera g_camera;


GLuint loadTextureFromFileToGPU(const std::string &filename) {
// Loading the image in CPU memory using stb_image
  int width, height, numComponents;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &numComponents, 0);
  GLuint texID; // OpenGL texture identifier
  glGenTextures(1, &texID); // generate an OpenGL texture container
  glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
  // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Fill the GPU texture with the data stored in the CPU image
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

return texID;
}


// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(action == GLFW_PRESS && key == GLFW_KEY_W) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if(action == GLFW_PRESS && key == GLFW_KEY_F) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
   else if(action == GLFW_PRESS && key == GLFW_KEY_O) {
    g_camera.zoomIn();
  } else if(action == GLFW_PRESS && key == GLFW_KEY_P) {
    g_camera.zoomOut();
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
    glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
  }
}

void errorCallback(int error, const char *desc) {
  std::cout <<  "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
  glfwSetErrorCallback(errorCallback);

  // Initialize GLFW, the library responsible for window management
  if(!glfwInit()) {
    std::cerr << "ERROR: Failed to init GLFW" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Before creating the window, set some option flags
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For Mac OS X
  // Create the window
  g_window = glfwCreateWindow(
    1000, 1000,
    "Interactive 3D Applications (OpenGL) - Simple Solar System",
    nullptr, nullptr);
  if(!g_window) {
    std::cerr << "ERROR: Failed to open window" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
  glfwMakeContextCurrent(g_window);
  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  glfwSetKeyCallback(g_window, keyCallback);
}

void initOpenGL() {
  // Load extensions for modern OpenGL
  if(!gladLoadGL(glfwGetProcAddress)) {
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);  // Enable the z-buffer test in the rasterization
  glClearColor(0, 0, 0, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename) {
  std::ifstream t(filename.c_str());
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
  const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
  glCompileShader(shader);
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
  }
  glAttachShader(program, shader);
  glDeleteShader(shader);
}

void initGPUprogram() {
  g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
  loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

  glUseProgram(g_program);
  //setting shader variables and textures
  g_earthTexID = loadTextureFromFileToGPU("media/earth.jpg");
  glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0); // texture unit 0
  g_moonTexID = loadTextureFromFileToGPU("media/moon.jpg");
  glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0); // texture unit 0
  g_mercuryTextID = loadTextureFromFileToGPU("media/mercury.jpg");
  glUniform1i(glGetUniformLocation(g_program, "material.albedoTex"), 0); // texture unit 0
}


class Mesh {
  public:
  void init();
  void render();
  static std::shared_ptr<Mesh> genSphere(const size_t resolution=16);

  glm::mat4 getMeshMatrix() const {return g_mesh;}
  void setMeshMatrix(const glm::mat4& newMatrix) {g_mesh = newMatrix;}
  
  private:
  std::vector<float> m_vertexPositions;
  std::vector<float> m_vertexNormals;
  std::vector<unsigned int> m_triangleIndices;
  GLuint m_vao = 0;
  GLuint m_posVbo = 0;
  GLuint m_normalVbo = 0;
  GLuint m_ibo = 0;
  glm::mat4 g_mesh;
  std::vector<float> m_vertexTexCoords;
  GLuint m_texCoordVbo = 0;
};







std::shared_ptr<Mesh> Mesh::genSphere(const size_t resolution) {

    std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>();

    // stores the normal vectors of the vertices.
    std::vector<float> m_vertexNormals;

    // stores the coords of the vertices for texture
    std::vector<float> m_vertexTexCoords;

    size_t numVertices = (resolution + 1) * (resolution + 1);
    size_t numIndices = resolution * resolution * 6;


    sphereMesh->m_vertexPositions.resize(numVertices * 3);
    sphereMesh->m_triangleIndices.resize(numIndices);

    float radius = 1;
    size_t vertexIndex = 0;

    for (size_t i = 0; i <= resolution; ++i) {
        for (size_t j = 0; j <= resolution; ++j) {
            float theta = i * (2 * glm::pi<float>()) / resolution;
            float phi = j * glm::pi<float>() / resolution;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            sphereMesh->m_vertexPositions[vertexIndex++] = x;
            sphereMesh->m_vertexPositions[vertexIndex++] = y;
            sphereMesh->m_vertexPositions[vertexIndex++] = z;
        }
    }

    // triangles indices
    size_t indexIndex = 0;

    for (size_t i = 0; i < resolution; ++i) {
        for (size_t j = 0; j < resolution; ++j) {
            size_t v0 = i * (resolution + 1) + j;
            size_t v1 = v0 + 1;
            size_t v2 = (i + 1) * (resolution + 1) + j;
            size_t v3 = v2 + 1;

            // triangle 1
            sphereMesh->m_triangleIndices[indexIndex++] = v0;
            sphereMesh->m_triangleIndices[indexIndex++] = v2;
            sphereMesh->m_triangleIndices[indexIndex++] = v1;

            // triangle 2
            sphereMesh->m_triangleIndices[indexIndex++] = v1;
            sphereMesh->m_triangleIndices[indexIndex++] = v2;
            sphereMesh->m_triangleIndices[indexIndex++] = v3;
        }
    }


    // normals
    for (size_t i = 0; i < numVertices; ++i) {
        float x = sphereMesh->m_vertexPositions[i * 3];
        float y = sphereMesh->m_vertexPositions[i * 3 + 1];
        float z = sphereMesh->m_vertexPositions[i * 3 + 2];
        
        glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
        
        sphereMesh->m_vertexNormals.push_back(normal.x);
        sphereMesh->m_vertexNormals.push_back(normal.y);
        sphereMesh->m_vertexNormals.push_back(normal.z);
    }


  // vertices coordinates
  for (size_t i = 0; i <= resolution; ++i) {
      for (size_t j = 0; j <= resolution; ++j) {
          float theta = i * (2.0f * M_PI / resolution);
          float phi = j * (M_PI / resolution);

          float u = i / static_cast<float>(resolution);
          float v = j / static_cast<float>(resolution);

          sphereMesh->m_vertexTexCoords.push_back(u);
          sphereMesh->m_vertexTexCoords.push_back(v);
      }
  }


    sphereMesh->init();

    return sphereMesh;
}



void Mesh::init() {
    //config VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    //config VBO (sommets)
    glGenBuffers(1, &m_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexPositions.size() * sizeof(float), m_vertexPositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);


    //passing the normals
    glGenBuffers(1, &m_normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexNormals.size() * sizeof(float), m_vertexNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);    //location = 1 (layout(location = 1) in vec3 vNormal;
    glEnableVertexAttribArray(1);


    //passing the texCord
    glGenBuffers(1, &m_texCoordVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_texCoordVbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexTexCoords.size() * sizeof(float), m_vertexTexCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(2);




    //config IBO (triangles)
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangleIndices.size() * sizeof(unsigned int), m_triangleIndices.data(), GL_STATIC_DRAW);

    // free VAO
    glBindVertexArray(0);

}

void Mesh::render() {
    glBindVertexArray(m_vao);
    //drawing
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_triangleIndices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}





std::shared_ptr<Mesh> sunMesh; // pointeur vers un objet Mesh
std::shared_ptr<Mesh> earthMesh;
std::shared_ptr<Mesh> moonMesh;
std::shared_ptr<Mesh> mercuryMesh;

void initCPUgeometry() {
  // creation des trois spheres
  sunMesh = Mesh::genSphere(64);
  earthMesh = Mesh::genSphere(64);
  moonMesh = Mesh::genSphere(64);
  mercuryMesh = Mesh::genSphere(64);
}



void initGPUgeometry() {

  g_sun = glm::mat4(1.0f); // Identity matrix, we set the Sun at the origin.
  g_earth = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));
  g_moon = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth + kRadOrbitMoon, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));
  g_mercury = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth/2, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth/2));

//animation

  sunMesh->init(); // init the spheres
  earthMesh->init();
  moonMesh->init();
  mercuryMesh->init();

}

void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  g_camera.setPosition(glm::vec3(0.0, 0.0, 15.0));
  g_camera.setNear(0.1);
  g_camera.setFar(80.1);
}

void init() {
  initGLFW();
  initOpenGL();
  initCPUgeometry();
  initGPUprogram();
  initGPUgeometry();
  initCamera();
}

void clear() {
  glDeleteProgram(g_program);

  glfwDestroyWindow(g_window);
  glfwTerminate();
}





// The main rendering call
void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
  const glm::vec3 camPosition = g_camera.getPosition();
  glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

  glBindVertexArray(g_vao);     // activate the VAO storing geometry data


glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_sun));
glUniform1i(glGetUniformLocation(g_program, "isSun"), 1); //the isSun variable is used to notify the Shader that it should use a color instead of a texture
glUniform3f(glGetUniformLocation(g_program, "objectColor"), 1.0f, 1.0f, 0.0f);  // yellow

sunMesh->render(); // Render the Sun

//value for the animation
float earthRotationSpeed = 1.0f;  
float earthOrbitSpeed = 0.5f;
float moonOrbitSpeed = 2.0f;
float moonRotationSpeed = moonOrbitSpeed;

float time = glfwGetTime();

glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_earth));
glUniform1i(glGetUniformLocation(g_program, "isSun"), 0);
glUniform3f(glGetUniformLocation(g_program, "objectColor"), 0.0f, 1.0f, 0.0f);  // green, but we don't need it here, because a texture will be aplied


// Transformations for the earth
glm::mat4 earthTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth * cos(0.3f * time), 0.0f, kRadOrbitEarth * sin(0.3f * time)));
glm::mat4 earthScale = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth));
glm::mat4 earthRotate = glm::rotate(glm::mat4(1.0f), 0.6f * time, glm::vec3(sin(glm::radians(23.5)), cos(glm::radians(23.5)), 0.0f));
g_earth = earthTranslate * earthScale * earthRotate;
glm::mat4 earthNormalMat = glm::transpose(glm::inverse(g_earth));
GLint earthNormalMatLoc = glGetUniformLocation(g_program, "normalMat");
glUniformMatrix4fv(earthNormalMatLoc, 1, GL_FALSE, glm::value_ptr(earthNormalMat));



glActiveTexture(GL_TEXTURE0); // activate texture unit 0
glBindTexture(GL_TEXTURE_2D, g_earthTexID);

earthMesh->render(); // Render the Earth



// transformations for the moon
glm::mat4 moonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitMoon * cos(0.6 * time), 0.0f, kRadOrbitMoon * sin(0.6 * time)));
glm::mat4 moonRotate = glm::rotate(glm::mat4(1.0f), 0.6f * time, glm::vec3(0, 1, 0.0f));
glm::mat4 moonScale = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeMoon));
g_moon = earthTranslate * moonTranslate * moonRotate * moonScale;


glm::mat4 moonNormalMat = glm::transpose(glm::inverse(g_moon));
GLint moonNormalMatLoc = glGetUniformLocation(g_program, "normalMat");
glUniformMatrix4fv(moonNormalMatLoc, 1, GL_FALSE, glm::value_ptr(moonNormalMat));

glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_moon));
glUniform3f(glGetUniformLocation(g_program, "objectColor"), 0.0f, 0.0f, 1.0f);

glActiveTexture(GL_TEXTURE0); // activate texture unit 0
glBindTexture(GL_TEXTURE_2D, g_moonTexID);

moonMesh->render();



//mercury

glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_mercury));
glUniform3f(glGetUniformLocation(g_program, "objectColor"), 1.0f, 0.4f, 0.3f);  // yellow

g_mercury = glm::translate(glm::mat4(1.0f), glm::vec3(kRadOrbitEarth/2, 0, 0)) * glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth/2));
glm::mat4 mercuryTranslate = glm::translate(glm::mat4(1.0f), glm::vec3((kRadOrbitEarth/2) * cos(0.3f * time * 3), 0.0f, (kRadOrbitEarth/2) * sin(0.3f * time * 3)));
glm::mat4 mercuryRotate = glm::rotate(glm::mat4(1.0f), 0.6f * time , glm::vec3(sin(glm::radians(23.5)), cos(glm::radians(23.5)), 0.0f));
glm::mat4 mercuryScale = glm::scale(glm::mat4(1.0f), glm::vec3(kSizeEarth/1.5));

g_mercury = mercuryTranslate * mercuryRotate * mercuryScale;

glm::mat4 mercuryNormalMat = glm::transpose(glm::inverse(g_mercury));
GLint mercuryNormalMatLoc = glGetUniformLocation(g_program, "normalMat");
glUniformMatrix4fv(mercuryNormalMatLoc, 1, GL_FALSE, glm::value_ptr(mercuryNormalMat));

glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(g_mercury));
glUniform3f(glGetUniformLocation(g_program, "objectColor"), 0.0f, 0.0f, 1.0f);

glActiveTexture(GL_TEXTURE0); // activate texture unit 0
glBindTexture(GL_TEXTURE_2D, g_mercuryTextID);
mercuryMesh->render();

glBindVertexArray(0);
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {

}

int main(int argc, char ** argv) {
  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
  while(!glfwWindowShouldClose(g_window)) {
    update(static_cast<float>(glfwGetTime()));
    render();
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}
