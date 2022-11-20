
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                   "}\0";

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int main(int argc, char *argv[]) {
  // Initialize Google’s logging & flags library.
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  LOG(INFO) << "glfw version: " << glfwGetVersionString();
  glfwSetErrorCallback([](int error, const char *description) {
    LOG(ERROR) << "glfw error: " << error << " " << description;
  });

  // glfw init
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // create a window and its opengl context
  GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    LOG(ERROR) << "Failed to create GLFW window";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // load opengl functions, which requires a current context to load from,
  // i.e., `glfwMakeContextCurrent(window)` be called before
  if (!gladLoadGL(glfwGetProcAddress)) {
    LOG(ERROR) << "Failed to initialize GLAD";
    return -1;
  }
  LOG(INFO) << "gl version: " << glGetString(GL_VERSION);

  // rendering size
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    VLOG(1) << "framebuffer_size_callback, new width " << width << " height " << height;
    glViewport(0, 0, width, height);
  });

  // ----- compile and link shaders -----
  // check status for shading program
  int success{0};
  char infoLog[512]{0};

  // vertex shader
  auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
    LOG(ERROR) << "ERROR::SHADER::VERTEX::COMPILATION_FAILED, " << infoLog;
  }

  // fragment shader
  auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
    LOG(ERROR) << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED, " << infoLog;
  }

  // shader program
  auto shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
    LOG(ERROR) << "ERROR::SHADER::PROGRAM::LINKING_FAILED, " << infoLog;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  // ----- compile and link shaders -----

  // vertices to draw
  float vertices[] = {-0.5f, -0.5f, 0.0f,  // vertex 0
                      0.5f,  -0.5f, 0.0f,  // vertex 1
                      0.0f,  0.5f,  0.0f}; // vertex 2
  unsigned int VBO{0};                     // vertex buffer objects
  glGenBuffers(1, &VBO);
  unsigned int VAO{0};
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); // bind Vertex Array Object
  // 0. copy to gpu memory
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // 1. then set the vertex attributes pointers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // rendering loop
  while (!glfwWindowShouldClose(window)) {
    // handle input
    processInput(window);

    // rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // check and call events and swap the buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
