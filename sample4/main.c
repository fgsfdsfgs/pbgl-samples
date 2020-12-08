#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>

#include <pbgl.h>
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>

#include <SDL.h>

// this provides an RGBA texutre, defined as variables
// texture_width, texture_height and texture_rgba[]
#include "texture.h"

static const GLfloat li_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
static const GLfloat li_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat li_position[] = { 0.0f, 0.0f, 2.0f, 1.0f };

static void load_textures(GLuint *texture)
{
    glGenTextures(3, texture); /* create three textures */
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_rgba);
    /* use no filtering */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /* the second texture */
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_rgba);
    /* use linear filtering */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /* the third texture */
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_rgba);
    // alternatively, instead of setting GL_GENERATE_MIPMAP before uploading level 0, you can do this afterwards:
    // glGenerateMipmap(GL_TEXTURE_2D);
    // but it will in most cases cause pbGL to reallocate the space for the texture
    /* use mipmapping */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void check_buttons(SDL_GameController *pad, GLboolean *pressed, GLboolean *held)
{
    SDL_GameControllerUpdate();
    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
      const GLboolean b = SDL_GameControllerGetButton(pad, i);
      pressed[i] = b && !held[i];
      held[i] = b;
    }
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    const GLboolean sdl_init = SDL_Init(SDL_INIT_GAMECONTROLLER) == 0;
    if (!sdl_init) {
        debugPrint("SDL_Init failed: %s\n", SDL_GetError());
        goto wait_then_cleanup;
    }

    if (SDL_NumJoysticks() < 1) {
        debugPrint("Please connect gamepad\n");
        goto wait_then_cleanup;
    }

    SDL_GameController *pad = SDL_GameControllerOpen(0);
    if (pad == NULL) {
        debugPrint("Failed to open gamecontroller 0\n");
        goto wait_then_cleanup;
    }

    const int err = pbgl_init(GL_TRUE);
    if (err < 0) {
        debugPrint("pbgl_init() failed: %d\n", err);
        goto wait_then_cleanup;
    }

    pbgl_set_swap_interval(1);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE); // FIXME: disable when the Z issue is fixed
    glDepthFunc(GL_LEQUAL);

    /* set up our lighting */
    glLightfv(GL_LIGHT1, GL_AMBIENT, li_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, li_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, li_position);
    glEnable(GL_LIGHT1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    GLuint textures[3];
    load_textures(textures);

    GLboolean light = GL_TRUE;
    GLboolean blend = GL_FALSE;
    GLfloat spd_x = 0.f;
    GLfloat spd_y = 0.f;
    GLfloat rot_x = 0.f;
    GLfloat rot_y = 0.f;
    GLfloat z = -5.f;
    GLuint filter = 0;

    GLboolean buttons_pressed[SDL_CONTROLLER_BUTTON_MAX] = { GL_FALSE };
    GLboolean buttons_held[SDL_CONTROLLER_BUTTON_MAX] = { GL_FALSE };

    while (GL_TRUE) {
        /* process input */

        check_buttons(pad, buttons_pressed, buttons_held);

        if (buttons_pressed[SDL_CONTROLLER_BUTTON_START])
            break;

        if (buttons_pressed[SDL_CONTROLLER_BUTTON_A]) {
            if (light)
                glDisable(GL_LIGHTING);
            else
                glEnable(GL_LIGHTING);
            light = !light;
        }

        if (buttons_pressed[SDL_CONTROLLER_BUTTON_B])
            filter = (filter + 1) % 3;

        if (buttons_held[SDL_CONTROLLER_BUTTON_Y])
            z -= 0.025f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_X])
            z += 0.025f;

        if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_LEFT])
            spd_y += 0.02f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_RIGHT])
            spd_y -= 0.02f;

        if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_UP])
            spd_x += 0.02f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_DOWN])
            spd_x -= 0.02f;

        if (buttons_pressed[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]) {
          if (blend) {
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE); // FIXME: disable when the Z issue is fixed
          } else {
            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
          }
          blend = !blend;
        }

        /* render */

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, z);
        glRotatef(rot_x, 1.0f, 0.0f, 0.0f); /* rotate on the X axis */
        glRotatef(rot_y, 0.0f, 1.0f, 0.0f); /* rotate on the Y axis */

        glBindTexture(GL_TEXTURE_2D, textures[filter]);  /* select our texture */

        glColor4f(1.f, 1.f, 1.f, 0.5f);

        glBegin(GL_QUADS);
            /* front face */
            glNormal3f(0.0f, 0.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f); 
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            /* back face */
            glNormal3f(0.0f, 0.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);
            /* right face */
            glNormal3f(1.0f, 0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, -1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            /* left face */
            glNormal3f(-1.0f, 0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, -1.0f);
            /* top face */
            glNormal3f(0.0f, 1.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, 1.0f, 1.0f); 
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            /* bottom face */
            glNormal3f(0.0f, -1.0f, 0.0f);
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
        rot_x += spd_x;
        rot_y += spd_y;

        pbgl_swap_buffers();
    }

wait_then_cleanup:
    Sleep(5000);
    pbgl_shutdown();
    SDL_Quit();

    return 0;
}
