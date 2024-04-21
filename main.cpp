#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


#include "meshes.h"

#include "camera.h" // Camera class

#define STB_IMAGE_IMPLEMENTATION


#include "stb_image.h"

using namespace std; // Uses the standard namespace

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "opengl"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;     // Handle for the vertex array object
        GLuint vbo;     // Handle for the vertex buffer object
        GLuint nvertices;   // Number of vertices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Shader program
    GLuint gProgramId;
}

double scrollY = 0.0f;
double _previous_scrolly_ = 0.0f;

// camera
Camera gCamera(glm::vec3(0.0f, 4.0f, 7.0f));
float gLastX = WINDOW_WIDTH / 2.0f;
float gLastY = WINDOW_HEIGHT / 2.0f;
bool gFirstMouse = true;


bool Increase = 0;
bool Decrease = 0;


// timing
float gDeltaTime = 0.0f; // time between current frame and last frame
float gLastFrame = 0.0f;



unsigned int TextureId;
unsigned int TextureId2;
unsigned int TextureId3;

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods); void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);




// Vertex Shader Program Source Code
const char* vertexShaderSource = R"(
  #version 400 core
  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec3 normal;
  layout (location = 2) in vec2 Tex;
       

  out vec2 TexCoord;
  uniform mat4 Proj;
  uniform mat4 Model;
  uniform mat4 View;

out vec3 vertexNormal; // For incoming normals
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader


  void main()
  {
     TexCoord = vec2(Tex);
     gl_Position = Proj * View * Model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	 
     vertexNormal = mat3(transpose(inverse(Model))) * normal; // get normal vectors in world space only and exclude normal translation properties
	 
     vertexFragmentPos = vec3(Model * vec4(aPos, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)
}
)";


// Fragment Shader Program Source Code
const char* fragmentShaderSource = R"(#version 400 core
out vec4 FragColor;
  in vec2 TexCoord;

 uniform sampler2D m_texture;

uniform vec3 color;

in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position

// Uniform / Global variables for  light color, light position, and camera/view position

uniform vec3 lightColor;

uniform vec3 lightPos;

uniform vec3 viewPosition;

vec3 phongLight(vec3 mlightColor, vec3 mlightPosition)
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

   //Calculate Ambient lighting*/
	float ambientStrength = 0.1f; // Set ambient or global lighting strength
	vec3 ambient = ambientStrength * mlightColor; // Generate ambient light color

	//Calculate Diffuse lighting*/
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
	vec3 lightDirection = normalize(mlightPosition - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse = impact * mlightColor; // Generate diffuse light color

	//Calculate Specular lighting*/
	float specularIntensity = 0.8f; // Set specular light strength
	float highlightSize = 16.0f; // Set specular highlight size
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * mlightColor;

	// Calculate phong result
	vec3 phong = (ambient + diffuse + specular);
	return phong;
}

void main()
{
    vec3 phong = phongLight(lightColor, lightPos);
    FragColor = vec4(phong, 1.0f) * texture(m_texture, TexCoord);
}
)";

glm::vec3 KeyLightColor(1.0f, 1.0f, 0.6f);

// Light position and scale
glm::vec3 KeyLightPos(1.6f, 5.45f, 3.2f);

Meshes Objects;

int Ploc;
int MMloc;
int Vloc;

// main function. Entry point to the OpenGL program
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    Objects.CreateMeshes();

    // Create the shader program
    UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId);

    // Load texture
    const char* texFilename = "images.jpg";
    if (!UCreateTexture(texFilename, TextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "screen.jpg";
    if (!UCreateTexture(texFilename, TextureId2))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "handle.jpg";
    if (!UCreateTexture(texFilename, TextureId3))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "m_texture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------

    glm::mat4  Projection = glm::infinitePerspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f);
    glm::mat4 View = glm::lookAt(glm::vec3(0.0f, 4.0f, -20.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0, 1.0f, 0.0f));

    // glm::mat4  Projection = glm::infinitePerspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f);
    // glm::mat4 View = glm::lookAt(glm::vec3(0.0f, -2.0f, -4.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    //     glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0, 1.0f, 0.0f));

    Ploc = glGetUniformLocation(gProgramId, "Proj");
    MMloc = glGetUniformLocation(gProgramId, "Model");
    Vloc = glGetUniformLocation(gProgramId, "View");

    glUniformMatrix4fv(Ploc, 1, false, &Projection[0][0]);
    glUniformMatrix4fv(MMloc, 1, false, &model[0][0]);
    glUniformMatrix4fv(Vloc, 1, false, &View[0][0]);

    while (!glfwWindowShouldClose(gWindow))
    {

        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;


        // input
        // -----
        UProcessInput(gWindow);

        if (Increase)
        {
            gCamera.MovementSpeed += 0.75f;
        }

        else if (Decrease)
        {
            gCamera.MovementSpeed -= 0.75f;
        }

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);



    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);


    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

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
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
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


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{

    {
        scrollY = yoffset;

        if (scrollY > _previous_scrolly_)
        {
            Increase = true;
            Decrease = false;
        }
        else if (scrollY < _previous_scrolly_)
        {
            Decrease = true;
            Increase = false;
        }

        _previous_scrolly_ = yoffset;
    }
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
    
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{
    // Clear the background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramId);


    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");

    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");

    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");



    glUniform3f(lightColorLoc, KeyLightColor.r, KeyLightColor.g, KeyLightColor.b);

    glUniform3f(lightPositionLoc, KeyLightPos.x, KeyLightPos.y, KeyLightPos.z);

    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    int colorLoc = glGetUniformLocation(gProgramId, "color");


    // Set the shader to be used

    glBindTexture(GL_TEXTURE_2D, TextureId);

    glUniformMatrix4fv(Vloc, 1, false, &gCamera.GetViewMatrix()[0][0]);

    glBindVertexArray(Objects.gBoxMesh.vao);

    //Desk

    glm::mat4 model =  glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 0.125f, 20.0f));

    glUniformMatrix4fv(MMloc, 1, false, &model[0][0]);
    glUniform3f(colorLoc, 0.65f, 0.65f, 0.65f);

    glDrawElements(GL_TRIANGLES, Objects.gBoxMesh.nIndices, GL_UNSIGNED_INT, nullptr);


    //Monitor
    glBindTexture(GL_TEXTURE_2D, TextureId2);

     model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 1.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 1.6875f, 0.1f));

    glUniformMatrix4fv(MMloc, 1, false, &model[0][0]);
    glUniform3f(colorLoc, 0.1f, 0.1f, 0.1f);

    glDrawElements(GL_TRIANGLES, Objects.gBoxMesh.nIndices, GL_UNSIGNED_INT, nullptr);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    glBindTexture(GL_TEXTURE_2D, TextureId3);
    //Cylinders
    glBindVertexArray(Objects.gCylinderMesh.vao);


    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.9f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 1.0f, 0.1f));

    glUniformMatrix4fv(MMloc, 1, false, &model[0][0]);
    glUniform3f(colorLoc, .5f, 0.5f, 0.35f);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
    glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
    glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides


    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // Draws the triangle
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nvertices); // Draws the triangle

    // Deactivate the VAO
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        //Positions              //Normals                Tex Coords
        // -----------------    -------------------------------------
       -0.5f,-0.5f,  0.5f,      1.0f, 0.0f, -1.0f,     0.0f, 0.0f,
       -0.5f,-0.5f, -0.5f,      0.0f, 0.0f, -1.0f,     1.0f, 0.0f,
        0.0f, 0.5f,  0.0f,      1.0f, 1.0f, -1.0f,     0.0f, 1.0f,

        0.5f,-0.5f, -0.5f,      0.0f, 0.0f,  1.0f,     0.0f, 0.0f,
        0.5f,-0.5f,  0.5f,      1.0f, 0.0f,  1.0f,     1.0f, 0.0f,
        0.0f, 0.5f,  0.0f,      0.5f, 0.5f,  1.0f,     0.0f, 1.0f,


        -0.5f,-0.5f, -0.5f,     0.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.5f,-0.5f, -0.5f,      1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        0.0f, 0.5f,  0.0f,      0.5f, 0.5f, 0.0f,       0.0f, 1.0f,

       -0.5f,-0.5f,  0.5f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        0.5f,-0.5f,  0.5f,      1.0f, 0.0f, 0.0f,      1.0f, 0.0f,
        0.0f, 0.5f,  0.0f,      0.5f, 0.5f, 0.0f,      0.0f, 1.0f,

       -0.5f,-0.5f, -0.5f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        0.5f,-0.5f, -0.5f,      1.0f, 0.0f, 0.0f,      0.0f, 1.0f,
        0.5f,-0.5f,  0.5f,      1.0f, 1.0f, 0.0f,      1.0f, 0.0f,

       -0.5f,-0.5f, -0.5f,      0.0f, 0.0f, 0.0f,      1.0f, 0.0f,
       -0.5f,-0.5f,  0.5f,      0.0f, 1.0f, 0.0f,      0.0f, 1.0f,
        0.5f,-0.5f,  0.5f,      1.0f, 1.0f, 0.0f,      1.0f, 1.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nvertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
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

    return true;
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 4);
    if (image)
    {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);


        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}

