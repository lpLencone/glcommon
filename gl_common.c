#include "gl_common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static char const *shader_type_string(GLuint shader_type);

char *cstr_from_file(char const *filename)
{
    FILE *f      = NULL;
    char *buffer = NULL;

    f = fopen(filename, "r");
    if (f == NULL) {
        goto fail;
    }
    if (fseek(f, 0, SEEK_END) < 0) {
        goto fail;
    }

    long size = ftell(f);
    if (size < 0) {
        goto fail;
    }

    buffer = malloc(size + 1);
    if (buffer == NULL) {
        goto fail;
    }

    if (fseek(f, 0, SEEK_SET) < 0) {
        goto fail;
    }

    fread(buffer, 1, size, f);
    if (ferror(f)) {
        goto fail;
    }

    buffer[size] = '\0';

    if (f) {
        fclose(f);
        errno = 0;
    }
    return buffer;

fail:
    if (f) {
        int saved_errno = errno;
        fclose(f);
        errno = saved_errno;
    }
    if (buffer) {
        free(buffer);
    }
    return NULL;
}

void gl_message_callback(GLenum light, GLenum type, GLuint id, GLenum severity,
        GLsizei length, GLchar const *message, void const *userParam)
{
    (void) light;
    (void) id;
    (void) length;
    (void) userParam;

    fprintf(stderr,
            "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            severity, message);
}

void glfw_error_callback(int error, char const *description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void) window;
    glViewport(0, 0, width, height);
    // scr_width  = (size_t) width; // global scr_{width|height}
    // scr_height = (size_t) height;
}

void init_gl(GLFWwindow **out_window)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
    glfwWindowHint(GLFW_FLOATING, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(
            /* scr_width, scr_height, */ 800, 600, "Window", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Tell glfw to hide and capture the cursor (hold the cursor in the center
    // of the window)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetErrorCallback(glfw_error_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    /* glfwSetCursorPosCallback(window, mouse_callback); */
    /* glfwSetScrollCallback(window, scroll_callback); */

    glfwMakeContextCurrent(window);

    //-----------------------------------------------------------------
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    printf("Using GLEW %s\n", glewGetString(GLEW_VERSION));

    if (GLEW_ARB_debug_output) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(gl_message_callback, 0);
    } else {
        fprintf(stderr, "WARNING! GLEW_ARB_debug_output is not available\n");
    }

    // glViewport(0, 0, scr_width, scr_height);
    glEnable(GL_DEPTH_TEST);

    *out_window = window;
}

void compile_shader_source(
        char const *filename, GLuint shader_type, GLuint *out_shader)
{
    GLuint shader;
    shader = glCreateShader(shader_type);

    char         *shader_source = cstr_from_file(filename);
    GLchar const *source =
            shader_source; // suppress "muhhh not constant" warning
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
        char info_log[512];
        glGetShaderInfoLog(shader, info_log_length, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::%s::COMPILATION_FAILED: %s\n",
                shader_type_string(shader_type), info_log);
        exit(EXIT_FAILURE);
    }

    free(shader_source);
    *out_shader = shader;
}

void link_program(
        GLuint vertex_shader, GLuint fragment_shader, GLuint *out_program)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
        char info_log[512];
        glGetProgramInfoLog(program, info_log_length, NULL, info_log);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED: %s\n",
                info_log);
        exit(EXIT_FAILURE);
    }
    *out_program = program;
}

static char const *shader_type_string(GLuint shader_type)
{
    switch (shader_type) {
        case GL_VERTEX_SHADER:
            return "VERTEX";
        case GL_FRAGMENT_SHADER:
            return "FRAGMENT";
        default:
            fprintf(stderr, "Out of reach\n");
            exit(1);
    }
}
