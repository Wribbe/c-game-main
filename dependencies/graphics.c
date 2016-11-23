#include <glad/glad.h>
#include <graphics/graphics.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/utils.h"

void gen_buffers(
                 GLuint num_buffers,
                 GLuint * buffer,
                 Point_Data * data,
                 GLuint draw_type
                )
{
    /* Generate num_buffers to GLuint * buffer pointer. */

    // Generate number of buffers.
    glGenBuffers(num_buffers, buffer);
    // Bind buffer.
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);
    // Load data to buffer.
    size_t data_size = sizeof(float) * data->elements;
    glBufferData(GL_ARRAY_BUFFER, data_size, data->data, draw_type);
    // Unbind buffer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gen_vertex_arrays(
                       GLuint num_buffers,
                       GLuint * buffer,
                       GLuint * vbo_binds,
                       size_t num_vbos,
                       Attrib_Pointer_Info * enable_arrayp,
                       size_t num_arrayp
                      )
{
    /* Generate num_buffer vertex array buffers. Bind all the vbos in the
     * vbo_bind list. Enable all Vertex Attrib Array pointers in
     * enabled_arrayp.
     */

    // Generate vertex array buffers.
    glGenVertexArrays(num_buffers, buffer);

    // Bind the vertex array buffer.
    glBindVertexArray(*buffer);

    // Bind all vbos in vbo_binds.
    for (size_t i=0; i<num_vbos; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_binds[i]);
    }

    // Enable all vertex attribute array pointers in enabled_arrayp.
    for (size_t i=0; i<num_arrayp; i++) {
        Attrib_Pointer_Info * arrayp = &enable_arrayp[i];
        glEnableVertexAttribArray(arrayp->index);
        glVertexAttribPointer(arrayp->index,
                              arrayp->size,
                              GL_FLOAT,
                              GL_FALSE,
                              arrayp->stride,
                              (void *)arrayp->offset);
    }

    // Unbind the vertex array buffer.
    glBindVertexArray(0);
}

void create_shader(GLuint * shader, const char * source_filename)
{
    /* Create and return shader based on contents of file. */

    const char * error_base = "[!] create_shader: ";

    /* Determine the type based on the file extension. */
    GLuint type = 0;
    if (strstr(source_filename, VERTEX_ENDING) != NULL) {
        type = GL_VERTEX_SHADER;
    } else if (strstr(source_filename, FRAGMENT_ENDING) != NULL) {
        type = GL_FRAGMENT_SHADER;
    } else {
        fprintf(stderr, "%s Can't determine shader type of file: %s.\n",
                error_base,
                source_filename);
        exit(1);
    }

    *shader = glCreateShader(type);

    // Open and read size of file.
    long filesize = 0;
    FILE * filehandle = open_file(source_filename, &filesize);
    if (!filehandle) {
        fprintf(stderr, "%s Could not find shader source file: %s.\n",
                error_base,
                source_filename);
        exit(1);
    }

    char temp_buffer[filesize];
    char * string_p = temp_buffer;

    // Read data to temp_buffer.
    size_t read = read_file(temp_buffer, filesize, filehandle);
    if (!read) {
        fprintf(stderr, "%s No objects read from %s.\n",
                error_base,
                source_filename);
        exit(1);
    }

    glShaderSource(*shader, 1, &string_p, NULL);
    glCompileShader(*shader);
    checkShaderStatus(*shader);
}


void checkShaderStatus(GLuint shader) {
    GLint status = 0;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_TRUE) {
        return;
    }

    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

    char message_buffer[logSize+1];

    glGetShaderInfoLog(shader, logSize, NULL, message_buffer);
    printf("Error in shader compilation: %s\n", message_buffer);
}

GLuint shader_program;

void link_program(GLuint * program, GLuint * shaders, size_t size) {

    // Create program.
    *program = glCreateProgram();

    // Attach all shaders in the list.
    for (size_t i=0; i<size; i++) {
        glAttachShader(*program, shaders[i]);
    }

    // Link program.
    glLinkProgram(*program);
}
