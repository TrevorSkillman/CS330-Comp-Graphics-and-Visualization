// Trevor SKillman
// CS330 - Final Project
// Southern New Hampshire University
// Professor Kurt Diesch
// 12/09/2022


#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" // Camera class



using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Trevor Skillman Final Project"; // Macro for window title
    const int STRIDE = 7;
    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data from the given mesh
    struct Mesh
    {
        GLuint vao; // cylinder vao
        GLuint vbos[2]; // cylinder vbos
        GLuint numVert; // cylinder Vertices

        GLuint pVao; // plane vao
        GLuint pVbos; // plane vbos
        GLuint pVertices; // plane vertices

        GLuint pyrVao; // pyramid vao
        GLuint pyrVbo; // pyramid vbo
        GLuint pyrVert; // pyramid vertices

        GLuint bookVao; // book vao
        GLuint bookVbo; // book vbo
        GLuint bookVert; // book vertices

        GLuint rubxVao; // rubix cube vao
        GLuint rubxVbo; // rubix cube vbo
        GLuint rubxVert; // rubix cube vertices
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    Mesh gMesh;


    // the texture IDs
    GLuint gTextureId;      // pyramid texture
    GLuint gTextureIdP;     // plane texture
    GLuint gCylTexId;    //     cylinder texture
    GLuint gBookTexId;    // booklet texture
    GLuint gRubxTexId;   // Rubix cube texture



    // the shader program
    GLuint gPyrProgramId; // pyramid shader program
    GLuint gLampPrgId; // lamp shader program
    GLuint gLampId2;  // second lighting source shader program
    GLuint gProgramIdP; // plane shader program
	GLuint gCylinderId;  // cylinder shader program
    GLuint gBookProgramId;  // booklet or sticky notes i use at my workplace
	GLuint gRubxProgramId; //   rubicks cube that is on my desk at work

    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;
    //GLint gTexWrapMode = GL_CLAMP_TO_EDGE;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 8.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // fps
    float gLastFrame = 0.0f;

    // for the light color and objects
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);

    glm::vec3 gLightColor(1.0f, 0.9f, 1.2f);
    glm::vec3 gLightPos(4.0f, 5.5f, 3.0f);
    glm::vec3 gLightScale(0.3f);

    //  Booklet
    glm::vec3 gBookPosition(0.75f, -1.3f, -1.0f);
    glm::vec3 gBookScale(0.5f);

    // second light source that is stationary
    glm::vec3 gLightColor2(1.0f, 1.0f, 1.2f);
    glm::vec3 gLightPos2(-8.0f, 11.5f, 7.0f);
    glm::vec3 gLightScale2(1.3f);

    //  pyramid position
    glm::vec3 gPyrPosition(-2.0f, 0.81f, -0.5f);
    glm::vec3 gPyrScale(0.85f);

    //  plane position
    glm::vec3 gPositionP(0.0f, -0.5f, 0.0f);
    glm::vec3 gScaleP(3.0f);

    //  position of the cylinder
    glm::vec3 gPositionCylinder(-2.0f, -0.35f, -0.5f);
    glm::vec3 gScaleCylinder(3.0f);

    //  Rubix cube position
    glm::vec3 gRubixPosition(2.0f, -0.91f, 1.0f);
    glm::vec3 gRubixScale(0.3f);


    // lamp is or is not orbiting
    bool gLampOrbits = true;

    // ortho global
    bool ortho = false;
}


bool Initialize(int, char* [], GLFWwindow** window);
void ResizeWindow(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);
void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void CreateCylinder(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen);
void CreateMesh(Mesh& mesh);
void DestroyMesh(Mesh& mesh);
bool CreateTexture(const char* filename, GLuint& textureId);
void DestroyTexture(GLuint textureId);
void Rend();
bool CreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void DestroyShaderProgram(GLuint programId);



// ************************************* Vertex Shader Source *************************************
const GLchar* vertexSSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); //  clip coordinates

	vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // fragment position

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only 
    vertexTextureCoordinate = textureCoordinate;
}
);

// ************************************* Fragment Shader Source *************************************
const GLchar* fragmentSSource = GLSL(440,

    in vec3 vertexNormal; //  incoming normals
in vec3 vertexFragmentPos; //  incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; //  outgoing color to the GPU

//  global variables for the light source, textures, and position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightColor2;
uniform vec3 lightPos2;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // good for using multiple textures
uniform vec2 uvScale;

void main()
{
    float ambientStrength = 0.5f; // ambient strength
    vec3 ambient = ambientStrength * lightColor; // ambient light color

    //  Calculating diffuse lighting for the first light
    vec3 norm = normalize(vertexNormal); // normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // distance between light and source object
    float impact = max(dot(norm, lightDirection), 0.0);// diffuse impact by using dot product
    vec3 diffuse = impact * lightColor; // diffuse light color

    //  Calculating specular lighting for the first light
    float specularIntensity = 0.3f; // specular light strength
    float highlightSize = 2.0f; // specular highlight size 2.0f;
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;


    // ************************************************* Second Light Source *************************************************
    float ambientStrength2 = 0.1f; // ambient strength for the second light
    vec3 ambient2 = ambientStrength2 * lightColor2; // ambient light color

    //  Calculate Diffuse of second lighting
    vec3 lightDir2 = normalize(lightPos2 - vertexFragmentPos); // Distance between light and source object
    float lightImpact2 = max(dot(norm, lightDir2), 0.0);// diffues impact by using dot product
    vec3 lightDiffuse2 = lightImpact2 * lightColor2; // diffuse light color

    //  Calculate Specular of second lighting
    float specularIntensity2 = 0.5f; // specular light strength
    float highlightSize2 = 8.0f; // specular highlight size
    vec3 viewDir2 = normalize(viewPosition - vertexFragmentPos); //  view direction
    vec3 reflectDir2 = reflect(-lightDir2, norm);//  reflection vector
    //Calculate specular component
    float specularComponent2 = pow(max(dot(viewDir2, reflectDir2), 0.0), highlightSize2);
    vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

    // Calculate phong result
    vec3 objectColor = texture(uTexture, vertexTextureCoordinate).xyz;
    vec3 lightResult = (ambient + diffuse + specular);
    vec3 fillResult = (ambient2 + lightDiffuse2 + specular2);
    vec3 lightResult2 = lightResult + fillResult;
    vec3 phong = (lightResult2)*objectColor;


    fragmentColor = vec4(phong, 1.0f); // Send lighting results to GPU
}
);


// ************************************* Lamp Vertex Shader Source *************************************
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


// ************************************* Lamp Fragment Shader Source *************************************
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

// ********************************* Second Lamp Vertex Shader Source *********************************
const GLchar* lampVertexSSource2 = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


// ********************************* Second Lamp Fragment Shader Source *********************************
const GLchar* lampFragmentSSource2 = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white 
}
);



// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!Initialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    CreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!CreateShaderProgram(vertexSSource, fragmentSSource, gPyrProgramId)) //   pyramid
        return EXIT_FAILURE;

    if (!CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampPrgId)) //  lamp
        return EXIT_FAILURE;

    if (!CreateShaderProgram(vertexSSource, fragmentSSource, gProgramIdP))  //    plane
        return EXIT_FAILURE;

    if (!CreateShaderProgram(vertexSSource, fragmentSSource, gCylinderId))  //    cylinder
        return EXIT_FAILURE;

    if (!CreateShaderProgram(vertexSSource, fragmentSSource, gBookProgramId))  // booklet / sticky notes
        return EXIT_FAILURE;

    if (!CreateShaderProgram(vertexSSource, fragmentSSource, gRubxProgramId))  // rubix cube
        return EXIT_FAILURE;

    if (!CreateShaderProgram(lampVertexSSource2, lampFragmentSSource2, gLampId2))  // second lamp
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "red.png";   // pyramid
    if (!CreateTexture(texFilename, gTextureId))
    {
        cout << "Could not load the texture: " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "Space.png";              //  plane
    if (!CreateTexture(texFilename, gTextureIdP))
    {
        cout << "Could not load the texture: " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "silver.png";          //  Cylinder
    if (!CreateTexture(texFilename, gCylTexId))
    {
        cout << "Could not load the texture: " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "blue.png";               //  booklet
    if (!CreateTexture(texFilename, gBookTexId))
    {
        cout << "Could not load the texture: " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "rubicks2.png";               //   rubix cube
    if (!CreateTexture(texFilename, gRubxTexId))
    {
        cout << "Could not load the texture: " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // using pyramid program to apply the texture to it
    glUseProgram(gPyrProgramId);
    // texture unit 0
    glUniform1i(glGetUniformLocation(gPyrProgramId, "uTexture"), 0);

    // set background color of the window to black 
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    while (!glfwWindowShouldClose(gWindow))
    {
        // fps
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        ProcessInput(gWindow);

        // Render the frame
        Rend();

        glfwPollEvents();
    }

    // Release mesh data
    DestroyMesh(gMesh);

    // Release texture
    DestroyTexture(gTextureId);

    // Release shader programs
    DestroyShaderProgram(gPyrProgramId);
    DestroyShaderProgram(gLampPrgId);
    DestroyShaderProgram(gProgramIdP);
    DestroyShaderProgram(gCylinderId);
    DestroyShaderProgram(gBookProgramId);
    DestroyShaderProgram(gLampId2);
    DestroyShaderProgram(gRubxProgramId);
    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool Initialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW 
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, ResizeWindow);
    glfwSetCursorPosCallback(*window, MousePositionCallback);
    glfwSetScrollCallback(*window, MouseScrollCallback);


    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW //
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void ProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    // ortho projection is engaged when the letter "P" is pressed
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        gCamera.Position = glm::vec3(-2.5f, 5.2f, -1.5f);
        gCamera.Pitch = -100.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // resets the ortho projection when the letter "O" is pressed
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        gCamera.Position = glm::vec3(0.0f, 0.0f, 5.0f);
        gCamera.Pitch = 0.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);



    // Add stubs for Q/E Upward/Downward movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    // Pause and resume lamp orbiting
    static bool isPaused = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gLampOrbits)
        gLampOrbits = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gLampOrbits)
        gLampOrbits = false;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void ResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// any mouse movement is called
void MousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// whenever the mouse wheel is scrolled this is called
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}


//  creating cylinder
void CreateCylinder(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen) {
    const float TWO_PI = 2.0f * 3.1415926f;
    const float radiansPerSide = TWO_PI / numSides;

    int currentVertex = 0;

    verts[0] = 0.0f;        //0 
    verts[1] = halfLen;     //0 
    verts[2] = 0.0f;        //0 
    verts[3] = 0.0f;        //0 
    verts[4] = 0.0f;        //0     
    verts[5] = 0.0f;        //0 
    verts[6] = 1.0f;        //0 
    currentVertex++;
    verts[7] = 0.0f;        //1 
    verts[8] = -halfLen;    //1 
    verts[9] = 0.0f;        //1 
    verts[10] = 0.0f;       //1 
    verts[11] = 0.0f;       //1 
    verts[12] = 0.0f;       //1 
    verts[13] = 1.0f;       //1 
    currentVertex++;

    int currentTriangle = 0;
    for (int edge = 0; edge < numSides; edge++) {
        float theta = ((float)edge) * radiansPerSide;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    //x
        verts[currentVertex * STRIDE + 1] = halfLen;                //y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    //z
        verts[currentVertex * STRIDE + 3] = 1.0f;                   //r
        verts[currentVertex * STRIDE + 4] = 0.0f;                   //g
        verts[currentVertex * STRIDE + 5] = 0.0f;                   //b
        verts[currentVertex * STRIDE + 6] = 1.0f;                   //a
        currentVertex++;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    //x
        verts[currentVertex * STRIDE + 1] = -halfLen;               //y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    //z
        verts[currentVertex * STRIDE + 3] = 1.0f;                   //r
        verts[currentVertex * STRIDE + 4] = 0.0f;                   //g
        verts[currentVertex * STRIDE + 5] = 0.0f;                   //b
        verts[currentVertex * STRIDE + 6] = 1.0f;                   //a
        currentVertex++;


        if (edge > 0) {
            //top triangle
            indices[(3 * currentTriangle) + 0] = 0;
            indices[(3 * currentTriangle) + 1] = currentVertex - 4;
            indices[(3 * currentTriangle) + 2] = currentVertex - 2;
            currentTriangle++;

            //bottom triangle
            indices[(3 * currentTriangle) + 0] = 1;
            indices[(3 * currentTriangle) + 1] = currentVertex - 3;
            indices[(3 * currentTriangle) + 2] = currentVertex - 1;
            currentTriangle++;

            //triangle for 1/2 rectangular side
            indices[(3 * currentTriangle) + 0] = currentVertex - 4;
            indices[(3 * currentTriangle) + 1] = currentVertex - 3;
            indices[(3 * currentTriangle) + 2] = currentVertex - 1;
            currentTriangle++;

            //traingle for second 1/2 rectangular side
            indices[(3 * currentTriangle) + 0] = currentVertex - 1;
            indices[(3 * currentTriangle) + 1] = currentVertex - 2;
            indices[(3 * currentTriangle) + 2] = currentVertex - 4;
            currentTriangle++;
        }
    }

    indices[(3 * currentTriangle) + 0] = 0;
    indices[(3 * currentTriangle) + 1] = currentVertex - 2;
    indices[(3 * currentTriangle) + 2] = 2;
    currentTriangle++;
    indices[(3 * currentTriangle) + 0] = 1;
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;
    indices[(3 * currentTriangle) + 2] = 3;
    currentTriangle++;
    indices[(3 * currentTriangle) + 0] = currentVertex - 2;
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;
    indices[(3 * currentTriangle) + 2] = 3;
    currentTriangle++;
    indices[(3 * currentTriangle) + 0] = 3;
    indices[(3 * currentTriangle) + 1] = 2;
    indices[(3 * currentTriangle) + 2] = currentVertex - 2;
    currentTriangle++;
}




// Functioned called to render a frame
void Rend()
{
    // Lamp orbits around the origin
    const float angularVelocity = glm::radians(45.0f);
    if (!gLampOrbits)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPos, 1.0f);
        gLightPos.x = newPosition.x;
        gLightPos.y = newPosition.y;
        gLightPos.z = newPosition.z;
    }

    glm::mat4 project;
    glm::mat4 views;
    // setting the views and projections for the ortho
    if (ortho)
    {
        GLfloat orthoWidth = (GLfloat)WINDOW_WIDTH * 0.01f; // this will be 10 percent of the window width
        GLfloat orthoHeight = (GLfloat)WINDOW_HEIGHT * 0.01f; // 10 percent of the window height

        views = gCamera.GetViewMatrix();
        project = glm::ortho(-orthoWidth, orthoWidth, orthoHeight, -orthoHeight, 0.1f, 100.0f);
    }
    else
    {
        views = gCamera.GetViewMatrix();
        project = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // frame and z buffers are cleared
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the vao
    glBindVertexArray(gMesh.pyrVao);

    // ************************************************ Drawing Pyramid ************************************************
    glUseProgram(gPyrProgramId);

    // transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gPyrPosition) * glm::scale(gPyrScale);

    // camera transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gPyrProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gPyrProgramId, "view");
    GLint projLoc = glGetUniformLocation(gPyrProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gPyrProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gPyrProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gPyrProgramId, "lightPos");
    GLint fillColorLoc = glGetUniformLocation(gPyrProgramId, "fillColor");
    GLint fillPositionLoc = glGetUniformLocation(gPyrProgramId, "fillPos");
    GLint viewPositionLoc = glGetUniformLocation(gPyrProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPos.x, gLightPos.y, gLightPos.z);
    glUniform3f(fillColorLoc, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(fillPositionLoc, gLightPos2.x, gLightPos2.y, gLightPos2.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gPyrProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.pyrVert);



    // ***************************************** Drawing the Plane ************************************************

    glUseProgram(gPyrProgramId);

    glBindTexture(GL_TEXTURE_2D, gTextureIdP);
    glActiveTexture(GL_TEXTURE0);

    model = glm::translate(gPositionP) * glm::scale(gScaleP);

    modelLoc = glGetUniformLocation(gProgramIdP, "model");
    viewLoc = glGetUniformLocation(gProgramIdP, "view");
    projLoc = glGetUniformLocation(gProgramIdP, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.pVao);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.pVertices);




    // ***************************************** Drawing the Cylinder ************************************************
    glUseProgram(gPyrProgramId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gCylTexId);


    model = glm::translate(gPositionCylinder) * glm::scale(gScaleCylinder);

    modelLoc = glGetUniformLocation(gCylinderId, "model");
    viewLoc = glGetUniformLocation(gCylinderId, "view");
    projLoc = glGetUniformLocation(gCylinderId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(gMesh.vao);

    glDrawElements(GL_TRIANGLES, gMesh.numVert, GL_UNSIGNED_SHORT, NULL);





    // ************************************************ Drawing the booklet/sticky notes ************************************************

    glUseProgram(gPyrProgramId);
    glBindTexture(GL_TEXTURE_2D, gBookTexId);
    glActiveTexture(GL_TEXTURE0);

    model = glm::translate(gBookPosition) * glm::scale(gBookScale);

    modelLoc = glGetUniformLocation(gBookProgramId, "model");
    viewLoc = glGetUniformLocation(gBookProgramId, "view");
    projLoc = glGetUniformLocation(gBookProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.bookVao);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.bookVert);


    // ******************************************* Drawing the rubicks cube *******************************************

    glUseProgram(gPyrProgramId);
    glBindTexture(GL_TEXTURE_2D, gRubxTexId);
    glActiveTexture(GL_TEXTURE0);

    model = glm::translate(gRubixPosition) * glm::scale(gRubixScale);

    modelLoc = glGetUniformLocation(gRubxProgramId, "model");
    viewLoc = glGetUniformLocation(gRubxProgramId, "view");
    projLoc = glGetUniformLocation(gRubxProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(gMesh.rubxVao);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.rubxVert);



    // ************************************* Drawing the lamp ************************************************
    glUseProgram(gLampPrgId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPos) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampPrgId, "model");
    viewLoc = glGetUniformLocation(gLampPrgId, "view");
    projLoc = glGetUniformLocation(gLampPrgId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.numVert);


    //  ***************************************** Drawing the second lamp ************************************************
    glUseProgram(gLampId2);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPos2) * glm::scale(gLightScale2);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampId2, "model");
    viewLoc = glGetUniformLocation(gLampId2, "view");
    projLoc = glGetUniformLocation(gLampId2, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.numVert);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.


}


// Implements the UCreateMesh function
void CreateMesh(Mesh& mesh)
{
    const int NUM_SIDES = 100;
    const int NUM_VERTICES = STRIDE * (2 + (2 * NUM_SIDES));
    const int NUM_INDICES = 12 * NUM_SIDES;
    GLfloat verts[NUM_VERTICES];
    GLushort indices[NUM_INDICES];



    // Position and Color data
    GLfloat pyrVerts[] = {
        //Positions                Normals                Textures
       -0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,      0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,      1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,     0.0f,  0.0f, -1.0f,      0.5f, 1.0f,

       -0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,      0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,      1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,     0.0f,  0.0f,  1.0f,      0.5f, 1.0f,

       -0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,      0.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,      0.0f, 0.0f,
        0.0f,  0.5f,  0.0f,    -1.0f,  0.0f,  0.0f,      0.5f, 1.0f,

        0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,      0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,      0.0f, 0.0f,
        0.0f,  0.5f,  0.0f,     1.0f,  0.0f,  0.0f,      0.5f, 1.0f,

        0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,      1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,      1.0f, 0.0f,
        0.0f,  0.5f,  0.0f,     0.0f, -1.0f,  0.0f,      0.5f, 1.0f,

       -0.5f, -0.5f,  0.5f,     0.0f,  1.0f,  0.0f,      0.0f, 0.0f,
       -0.5f, -0.5f, -0.5f,     0.0f,  1.0f,  0.0f,      0.0f, 1.0f,
        0.0f,  0.5f,  0.0f,     0.0f,  1.0f,  0.0f,      0.5f, 1.0f


    };
    GLfloat planeVerts[] = {
        //Positions                      Normals                Textures
          2.0f, -0.2f,  1.0f,        0.0f,  0.0f, 1.0f,        1.0f, 0.0f,
         -2.0f, -0.2f, -1.0f,        0.0f,  0.0f, 1.0f,        0.0f, 0.0f,
          2.0f, -0.2f, -1.0f,        0.0f,  0.0f, 1.0f,        0.0f, 1.0f,

          2.0f, -0.2f,  1.0f,        0.0f,  0.0f, 1.0f,        1.0f, 0.0f,
         -2.0f, -0.2f,  1.0f,        0.0f,  0.0f, 1.0f,        1.0f, 1.0f,
         -2.0f, -0.2f, -1.0f,        0.0f,  0.0f, 1.0f,        0.0f, 0.0f,
    };

    GLfloat bookVerts[] = {
        //Positions                      Normals                Textures
         -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
          0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,


         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,


         -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


          0.5f,  0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,


         -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f
    };

    GLfloat rubixVerts[] = {
        //Positions                      Normals                Textures
         -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,
          0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,        0.0f, 0.0f,


         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,        0.0f, 0.0f,


         -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


          0.5f,  0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       1.0f,  0.0f,  0.0f,        1.0f, 0.0f,


         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,        0.0f, 1.0f,


         -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,        0.0f, 1.0f
    };


    //fill verts/indices arrays with data 
    CreateCylinder(verts, indices, NUM_SIDES, 0.15f, 0.25f);



    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.pyrVert = sizeof(pyrVerts) / (sizeof(pyrVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.pyrVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.pyrVao);

    // Create 1 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.pyrVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.pyrVbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyrVerts), pyrVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);






    //************************************* Plane *************************************//
    mesh.pVertices = sizeof(planeVerts) / (sizeof(planeVerts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.pVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.pVao);

    // Create 1 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.pVbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.pVbos); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);







    // ************************************** cylinder **************************************
    const GLuint floatsPerVertex2 = 3;
    const GLuint floatsPerColor2 = 4;
    const GLuint floatsPerUV2 = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.numVert = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride2 = sizeof(float) * (floatsPerVertex2 + floatsPerColor2);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex2, GL_FLOAT, GL_FALSE, stride2, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * floatsPerVertex2));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * (floatsPerVertex2 + floatsPerColor2)));
    glEnableVertexAttribArray(2);







    // ************************************** booklet or stickynotes *****************************************


    const GLuint bookFloatsPerVertex = 3;
    const GLuint bookFloatsPerNormal = 3;
    const GLuint bookFloatsPerUV = 2;

    mesh.bookVert = sizeof(bookVerts) / (sizeof(bookVerts[0]) * (bookFloatsPerVertex + bookFloatsPerNormal + bookFloatsPerUV));

    glGenVertexArrays(1, &mesh.bookVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.bookVao);

    // creating one buffer
    glGenBuffers(1, &mesh.bookVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.bookVbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(bookVerts), bookVerts, GL_STATIC_DRAW); // sending vertex coordinate to the GPU

    // Strides between vertex coordinates 
    GLint cubeStride = sizeof(float) * (bookFloatsPerVertex + bookFloatsPerNormal + bookFloatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, bookFloatsPerVertex, GL_FLOAT, GL_FALSE, cubeStride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, bookFloatsPerNormal, GL_FLOAT, GL_FALSE, cubeStride, (void*)(sizeof(float) * bookFloatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, bookFloatsPerUV, GL_FLOAT, GL_FALSE, cubeStride, (void*)(sizeof(float) * (bookFloatsPerVertex + bookFloatsPerNormal)));
    glEnableVertexAttribArray(2);





    // ************************* rubricks cube ***********************************************

    const GLuint rubixFloatsPerVertex = 3;
    const GLuint rubixFloatsPerNormal = 3;
    const GLuint rubixFloatsPerUV = 2;

    mesh.rubxVert = sizeof(rubixVerts) / (sizeof(rubixVerts[0]) * (rubixFloatsPerVertex + rubixFloatsPerNormal + rubixFloatsPerUV));

    glGenVertexArrays(1, &mesh.rubxVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.rubxVao);

    // Create 1 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.rubxVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.rubxVbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(rubixVerts), rubixVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates 
    GLint cube2Stride = sizeof(float) * (rubixFloatsPerVertex + rubixFloatsPerNormal + rubixFloatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, rubixFloatsPerVertex, GL_FLOAT, GL_FALSE, cube2Stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, rubixFloatsPerNormal, GL_FLOAT, GL_FALSE, cube2Stride, (void*)(sizeof(float) * rubixFloatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, rubixFloatsPerUV, GL_FLOAT, GL_FALSE, cube2Stride, (void*)(sizeof(float) * (rubixFloatsPerVertex + rubixFloatsPerNormal)));
    glEnableVertexAttribArray(2);

}


void DestroyMesh(Mesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.pyrVao);
    glDeleteBuffers(1, &mesh.pyrVbo);
}


/*Generate and load the texture*/
bool CreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void DestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}



bool CreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
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

    //  compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void DestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}