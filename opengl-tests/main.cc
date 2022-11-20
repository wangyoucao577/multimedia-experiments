
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

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
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
    VLOG(1) << "framebuffer_size_callback, new width " << width << " height " << height;
    glViewport(0, 0, width, height);
  });

  // rendering loop
  while (!glfwWindowShouldClose(window)) {
    // handle input
    processInput(window);

    // rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // check and call events and swap the buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
