#ifndef GL_GL_COMMON_H_
#define GL_GL_COMMON_H_

#include "GL/glew.h"
#include <GLFW/glfw3.h>

char *cstr_from_file(char const *filename);

void gl_message_callback(GLenum light, GLenum type, GLuint id, GLenum severity,
        GLsizei length, GLchar const *message, void const *userParam);

void glfw_error_callback(int error, char const *description);

void init_gl(GLFWwindow **out_window);

void compile_shader_source(
        char const *filename, GLuint shader_type, GLuint *out_shader);
void link_program(
        GLuint vertex_shader, GLuint fragment_shader, GLuint *out_program);

#endif // GL_GL_COMMON_H_
