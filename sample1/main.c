#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>

#include <pbgl.h>
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    const int err = pbgl_init(GL_TRUE);
    if (err < 0) {
        debugPrint("pbgl_init() failed: %d\n", err);
        Sleep(5000);
        XReboot();
    }

    pbgl_set_swap_interval(1);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    // automatically die in about 10 seconds
    GLuint frames = 0;
    while (frames++ < 60 * 10) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();

        glTranslatef(-1.5f, 0.0f, -6.0f);

        glBegin(GL_TRIANGLES);
            glColor3f(1.f, 0.f, 0.f);
            glVertex3f( 0.0f,  1.0f,  0.0f);
            glColor3f(0.f, 1.f, 0.f);
            glVertex3f(-1.0f, -1.0f,  0.0f);
            glColor3f(0.f, 0.f, 1.f);
            glVertex3f( 1.0f, -1.0f,  0.0f);
        glEnd();

        glTranslatef(3.0f, 0.0f, 0.0f);

        glColor3f(0.5f, 0.5f, 1.0f);
        glBegin(GL_QUADS);
            glVertex3f(-1.0f,  1.0f,  0.0f);
            glVertex3f( 1.0f,  1.0f,  0.0f);
            glVertex3f( 1.0f, -1.0f,  0.0f);
            glVertex3f(-1.0f, -1.0f,  0.0f);
        glEnd();

        pbgl_swap_buffers();
    }

    pbgl_shutdown();
    XReboot();

    return 0;
}
