#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>

#include <pbgl.h>
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>

// this provides an RGBA texutre, defined as variables
// texture_width, texture_height and texture_rgba[]
#include "texture.h"

static GLuint load_texture(GLuint w, GLuint h, GLenum extfmt, const void *data)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, extfmt, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    const int err = pbgl_init(GL_TRUE);
    if (err < 0) {
        debugPrint("pbgl_init() failed: %d\n", err);
        Sleep(5000);
        return 0;
    }

    pbgl_set_swap_interval(1);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glEnable(GL_CULL_FACE); // FIXME: disable when the Z issue is fixed
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    GLuint texture = load_texture(texture_width, texture_height, GL_RGBA, texture_rgba);

    GLfloat rot_x = 0.f;
    GLfloat rot_y = 0.f;
    GLfloat rot_z = 0.f;

    // automatically die in about 15 seconds
    GLuint frames = 0;
    while (frames++ < 60 * 15) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glRotatef(rot_x, 1.0f, 0.0f, 0.0f); /* rotate on the X axis */
        glRotatef(rot_y, 0.0f, 1.0f, 0.0f); /* rotate on the Y axis */
        glRotatef(rot_z, 0.0f, 0.0f, 1.0f); /* rotate on the Z axis */
        glTranslatef(0.0f, 0.0f, -5.0f);
        glBindTexture(GL_TEXTURE_2D, texture); /* select our texture */

        glBegin(GL_QUADS);
            /* front face */
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f); 
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            /* back face */
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);
            /* right face */
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, -1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            /* left face */
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f);
            /* top face */
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, 1.0f, 1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            /* bottom face */
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, -1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f);
        glEnd();

        /* change the rotation angles */
        rot_x += 0.3f;
        rot_y += 0.2f;
        rot_z += 0.4f;

        pbgl_swap_buffers();
    }

    pbgl_shutdown();

    return 0;
}
