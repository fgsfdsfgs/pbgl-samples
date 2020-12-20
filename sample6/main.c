#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>

#include <pbgl.h>
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>

#include <SDL.h>

#include "object.h"

#define GLOBJ_FILE "D:\\data\\object.txt"

typedef float vec4f[4]; 
typedef float mat4f[16];

static GLObject obj;

static float objPos[] = { -4.0f, -2.0f, -10.0f };
static float lightPos[] = {  0.0f,  5.0f, -4.0f,  1.0f };
static float rot[3] = { 0.f };
static float rotspeed[3] = { 0.1f, -0.1f };

static const float lightAmb[] = {  0.1f,  0.1f,  0.1f,  1.0f };
static const float lightDif[] = {  0.6f,  0.6f,  0.6f,  1.0f };
static const float lightSpc[] = { -0.2f, -0.2f, -0.2f,  1.0f };

static const float matAmb[] = { 0.4f, 0.4f, 0.4f, 1.0f };
static const float matDif[] = { 0.2f, 0.6f, 0.9f, 1.0f };
static const float matSpc[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static const float matShn[] = { 0.0f };

static inline void transform_vec(const mat4f m, vec4f v)
{
    vec4f res;
    res[0] = m[ 0]*v[0] + m[ 4]*v[1] + m[ 8]*v[2] + m[12]*v[3];
    res[1] = m[ 1]*v[0] + m[ 5]*v[1] + m[ 9]*v[2] + m[13]*v[3];
    res[2] = m[ 2]*v[0] + m[ 6]*v[1] + m[10]*v[2] + m[14]*v[3];
    res[3] = m[ 3]*v[0] + m[ 7]*v[1] + m[11]*v[2] + m[15]*v[3];
    v[0] = res[0];
    v[1] = res[1];
    v[2] = res[2];
    v[3] = res[3];
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

static inline void draw_room(void)
{
    glBegin(GL_QUADS); // Begin Drawing Quads
        // Floor
        glNormal3f(0.0f, 1.0f, 0.0f);     // Normal Pointing Up
        glVertex3f(-10.0f,-10.0f,-20.0f); // Back Left
        glVertex3f(-10.0f,-10.0f, 20.0f); // Front Left
        glVertex3f( 10.0f,-10.0f, 20.0f); // Front Right
        glVertex3f( 10.0f,-10.0f,-20.0f); // Back Right
        // Ceiling
        glNormal3f(0.0f,-1.0f, 0.0f);     // Normal Point Down
        glVertex3f(-10.0f, 10.0f, 20.0f); // Front Left
        glVertex3f(-10.0f, 10.0f,-20.0f); // Back Left
        glVertex3f( 10.0f, 10.0f,-20.0f); // Back Right
        glVertex3f( 10.0f, 10.0f, 20.0f); // Front Right
        // Front Wall
        glNormal3f(0.0f, 0.0f, 1.0f);     // Normal Pointing Away From Viewer
        glVertex3f(-10.0f, 10.0f,-20.0f); // Top Left
        glVertex3f(-10.0f,-10.0f,-20.0f); // Bottom Left
        glVertex3f( 10.0f,-10.0f,-20.0f); // Bottom Right
        glVertex3f( 10.0f, 10.0f,-20.0f); // Top Right
        // Back Wall
        glNormal3f(0.0f, 0.0f,-1.0f);     // Normal Pointing Towards Viewer
        glVertex3f( 10.0f, 10.0f, 20.0f); // Top Right
        glVertex3f( 10.0f,-10.0f, 20.0f); // Bottom Right
        glVertex3f(-10.0f,-10.0f, 20.0f); // Bottom Left
        glVertex3f(-10.0f, 10.0f, 20.0f); // Top Left
        // Left Wall
        glNormal3f(1.0f, 0.0f, 0.0f);     // Normal Pointing Right
        glVertex3f(-10.0f, 10.0f, 20.0f); // Top Front
        glVertex3f(-10.0f,-10.0f, 20.0f); // Bottom Front
        glVertex3f(-10.0f,-10.0f,-20.0f); // Bottom Back
        glVertex3f(-10.0f, 10.0f,-20.0f); // Top Back
        // Right Wall
        glNormal3f(-1.0f, 0.0f, 0.0f);    // Normal Pointing Left
        glVertex3f( 10.0f, 10.0f,-20.0f); // Top Back
        glVertex3f( 10.0f,-10.0f,-20.0f); // Bottom Back
        glVertex3f( 10.0f,-10.0f, 20.0f); // Bottom Front
        glVertex3f( 10.0f, 10.0f, 20.0f); // Top Front
    glEnd();
}

static void draw_scene(void)
{
    mat4f mvinv;
    vec4f wlp, lp;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    /* calculate the light's position in the object's local coordinate system                */
    /* by transforming its world position with the inverse of the object's modelview matrix  */
    /* to do this, build modelview matrix by applying transformations in the reverse order   */
    /* with inverted parameters and then get its value                                       */

    glLoadIdentity();

    glRotatef(-rot[1], 0.0f, 1.0f, 0.0f);
    glRotatef(-rot[0], 1.0f, 0.0f, 0.0f);

    glGetFloatv(GL_MODELVIEW_MATRIX, mvinv);

    lp[0] = lightPos[0];
    lp[1] = lightPos[1];
    lp[2] = lightPos[2];
    lp[3] = lightPos[3];
    transform_vec(mvinv, lp);

    glTranslatef(-objPos[0], -objPos[1], -objPos[2]);

    glGetFloatv(GL_MODELVIEW_MATRIX, mvinv);

    wlp[0] = wlp[1] = wlp[2] = 0.0f;
    wlp[3] = 1.f;
    transform_vec(mvinv, wlp);

    lp[0] += wlp[0];
    lp[1] += wlp[1];
    lp[2] += wlp[2];

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glLoadIdentity();
    glTranslatef(0.f, 0.f, -10.f);
    // GL_POSITION is transformed by the current modelview matrix
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos);

    /* draw the room */

    draw_room();

    /* draw the object and its shadow */

    glTranslatef(objPos[0], objPos[1], objPos[2]);
    glRotatef(rot[0], 1.0f, 0.0f, 0.0f);
    glRotatef(rot[1], 0.0f, 1.0f, 0.0f);

    glColor3f(0.5f, 0.0f, 1.0f);
    globj_draw(&obj);
    globj_cast_shadow(&obj, lp);

    /* mark light position */

    glLoadIdentity();
    glTranslatef(lightPos[0], lightPos[1], lightPos[2] - 10.f);
    glColor4f(0.7f, 0.4f, 0.0f, 1.0f);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
        glVertex3f(-0.25f, -0.25f, 0.f);
        glVertex3f(-0.25f, +0.25f, 0.f);
        glVertex3f(+0.25f, +0.25f, 0.f);
        glVertex3f(+0.25f, -0.25f, 0.f);
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

static GLboolean load_globj(GLObject *dst, const char *fname)
{
    if (!globj_read(dst, fname))
        return GL_FALSE;

    globj_set_connectivity(dst);

    for (GLuint i = 0; i < dst->nPlanes; ++i)
        globj_calc_plane(dst, &(dst->planes[i]));

    return GL_TRUE;
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

    if (!load_globj(&obj, GLOBJ_FILE)) {
        debugPrint("Failed to load %s\n", GLOBJ_FILE);
        goto wait_then_cleanup;
    }

    const int err = pbgl_init(GL_TRUE);
    if (err < 0) {
        debugPrint("pbgl_init() failed: %d\n", err);
        goto wait_then_cleanup;
    }

    pbgl_set_swap_interval(1);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
    glClearDepth(1.0f);
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDif);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpc);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);

    // FIXME: this doesn't actually do anything in pbGL
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpc);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShn);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glViewport(0, 0, 640, 480);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.001, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLboolean buttons_pressed[SDL_CONTROLLER_BUTTON_MAX] = { GL_FALSE };
    GLboolean buttons_held[SDL_CONTROLLER_BUTTON_MAX] = { GL_FALSE };

    while (GL_TRUE) {
        /* process input */

        check_buttons(pad, buttons_pressed, buttons_held);

        if (buttons_pressed[SDL_CONTROLLER_BUTTON_START])
            break;

        if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_LEFT])
            rotspeed[1] += 0.1f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_RIGHT])
            rotspeed[1] -= 0.1f;
        if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_UP])
            rotspeed[0] += 0.1f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_DOWN])
            rotspeed[0] -= 0.1f;

        if (buttons_held[SDL_CONTROLLER_BUTTON_Y])
            objPos[2] -= 0.1f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_A])
            objPos[2] += 0.1f;
        if (buttons_held[SDL_CONTROLLER_BUTTON_B])
            objPos[0] -= 0.1f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_X])
            objPos[0] += 0.1f;
        if (buttons_held[SDL_CONTROLLER_BUTTON_LEFTSHOULDER])
            objPos[1] -= 0.1f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER])
            objPos[1] += 0.1f;


        rot[0] += rotspeed[0];
        rot[1] += rotspeed[1];
        rot[2] += rotspeed[2];

        /* render */

        draw_scene();

        pbgl_swap_buffers();
    }

wait_then_cleanup:
    Sleep(5000);
    pbgl_shutdown();
    SDL_Quit();

    return 0;
}
