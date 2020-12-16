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

static const GLfloat cube_vbuf[] = {
    // position, texcoord, normal
    // first face (PZ)
    // first triangle
    -0.5f, -0.5f, +0.5f,   0.0f, 0.0f,   0.0f, 0.0f, +1.0f,
    +0.5f, -0.5f, +0.5f,   1.0f, 0.0f,   0.0f, 0.0f, +1.0f,
    +0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   0.0f, 0.0f, +1.0f,
    // second triangle
    +0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   0.0f, 0.0f, +1.0f,
    -0.5f, +0.5f, +0.5f,   0.0f, 1.0f,   0.0f, 0.0f, +1.0f,
    -0.5f, -0.5f, +0.5f,   0.0f, 0.0f,   0.0f, 0.0f, +1.0f,

    // second face (MZ)
    // first triangle
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
    -0.5f, +0.5f, -0.5f,   1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
    +0.5f, +0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    // second triangle
    +0.5f, +0.5f, -0.5f,   1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    +0.5f, -0.5f, -0.5f,   0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, 0.0f, -1.0f,

    // third face (PX)
    // first triangle
    +0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   +1.0f, 0.0f, 0.0f,
    +0.5f, +0.5f, -0.5f,   1.0f, 0.0f,   +1.0f, 0.0f, 0.0f,
    +0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   +1.0f, 0.0f, 0.0f,
    // second triangle
    +0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   +1.0f, 0.0f, 0.0f,
    +0.5f, -0.5f, +0.5f,   0.0f, 1.0f,   +1.0f, 0.0f, 0.0f,
    +0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   +1.0f, 0.0f, 0.0f,

    // fourth face (MX)
    // first triangle
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, +0.5f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
    // second triangle
    -0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, +0.5f, -0.5f,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

    // fifth face (PY)
    // first triangle
    -0.5f, +0.5f, -0.5f,   0.0f, 0.0f,   0.0f, +1.0f, 0.0f,
    -0.5f, +0.5f, +0.5f,   1.0f, 0.0f,   0.0f, +1.0f, 0.0f,
    +0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   0.0f, +1.0f, 0.0f,
    // second triangle
    +0.5f, +0.5f, +0.5f,   1.0f, 1.0f,   0.0f, +1.0f, 0.0f,
    +0.5f, +0.5f, -0.5f,   0.0f, 1.0f,   0.0f, +1.0f, 0.0f,
    -0.5f, +0.5f, -0.5f,   0.0f, 0.0f,   0.0f, +1.0f, 0.0f,

    // sixth face (MY)
    // first triangle
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    +0.5f, -0.5f, -0.5f,   1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
    +0.5f, -0.5f, +0.5f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    // second triangle
    +0.5f, -0.5f, +0.5f,   1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, +0.5f,   0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
};

const GLushort cube_indices[] = {
     0,  1,  2,  3,  4,  5,  6,
     7,  8,  9, 10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27,
    28, 29, 30, 31, 32, 33, 34,
    35
};

static const GLuint cube_stride = (3 + 2 + 3) * sizeof(GLfloat);
static const GLuint cube_numverts = 36;

static const GLfloat li_ambient[][4] = {
  { 0.1f, 0.1f, 0.1f, 1.0f },
  { 0.1f, 0.1f, 0.1f, 1.0f },
  { 0.1f, 0.1f, 0.1f, 1.0f },
  { 0.1f, 0.1f, 0.1f, 1.0f },
};
static const GLfloat li_diffuse[][4] = {
  { 1.0f, 0.0f, 0.0f, 1.0f },
  { 0.0f, 1.0f, 0.0f, 1.0f },
  { 0.0f, 0.0f, 1.0f, 1.0f },
  { 0.0f, 1.0f, 1.0f, 1.0f },
};
static const GLfloat li_position[][4] = {
  { -1.0f,  0.0f,  0.0f, 0.0f },
  { +1.0f,  0.0f,  0.0f, 0.0f },
  {  0.0f,  0.0f, +1.0f, 0.0f },
  {  0.0f,  0.0f, -1.0f, 0.0f },
};
static const GLfloat li_ambient_scene[4] = { 0.f };

static void check_buttons(SDL_GameController *pad, GLboolean *pressed, GLboolean *held)
{
    SDL_GameControllerUpdate();
    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i) {
      const GLboolean b = SDL_GameControllerGetButton(pad, i);
      pressed[i] = b && !held[i];
      held[i] = b;
    }
}

static GLuint get_texture(const GLuint w, const GLuint h)
{
    GLubyte *buf = malloc(w * h * 3);
    if (!buf) return 0;

    GLubyte *ptr = buf;
    for (GLuint y = 0; y < h; ++y) {
        for (GLuint x = 0; x < w; ++x) {
            const GLubyte v = x ^ y;
            *(ptr++) = v;
            *(ptr++) = v;
            *(ptr++) = v;
        }
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

    return tex;
}

static inline void draw_immediate(GLenum prim, GLuint numverts, GLuint stride, const GLfloat *buf)
{
    stride /= sizeof(GLfloat);
    glBegin(prim);
    for (GLuint i = 0; i < cube_numverts; ++i)
    {
        glTexCoord2fv(buf + 3);
        glNormal3fv(buf + 5);
        glVertex3fv(buf);
        buf += stride;
    }
    glEnd();
}

int main(void)
{
    GLfloat *gpu_buf = NULL;

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

    // vertex data has to be in contiguous memory
    gpu_buf = pbgl_alloc(sizeof(cube_vbuf), GL_FALSE);
    if (!gpu_buf) {
        debugPrint("Failed to allocate %u bytes\n", sizeof(cube_vbuf));
        goto wait_then_cleanup;
    }
    memcpy(gpu_buf, cube_vbuf, sizeof(cube_vbuf));

    const int err = pbgl_init(GL_TRUE);
    if (err < 0) {
        debugPrint("pbgl_init() failed: %d\n", err);
        goto wait_then_cleanup;
    }

    pbgl_set_swap_interval(1);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE); // FIXME: disable when the Z issue is fixed
    glDepthFunc(GL_LEQUAL);

    /* set up our lighting */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, li_ambient_scene);
    for (GLuint i = 0; i < sizeof(li_ambient) / sizeof(li_ambient[0]); ++i) {
        glLightfv(GL_LIGHT1 + i, GL_AMBIENT, li_ambient[i]);
        glLightfv(GL_LIGHT1 + i, GL_DIFFUSE, li_diffuse[i]);
        glLightfv(GL_LIGHT1 + i, GL_POSITION, li_position[i]);
        glEnable(GL_LIGHT1 + i);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 640.0 / 480.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    GLboolean light = GL_TRUE;
    GLboolean blend = GL_FALSE;
    GLfloat cam_rot[3] = { 0.f, 0.f, 0.f };
    GLfloat cam_dist = -5.f;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, cube_stride, gpu_buf);
    glTexCoordPointer(2, GL_FLOAT, cube_stride, gpu_buf + 3);
    glNormalPointer(GL_FLOAT, cube_stride, gpu_buf + 3 + 2);

    GLuint tex = get_texture(128, 128);

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

        if (buttons_held[SDL_CONTROLLER_BUTTON_Y])
            cam_dist -= 0.1f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_X])
            cam_dist += 0.1f;

        if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_LEFT])
            cam_rot[1] += 0.35f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_RIGHT])
            cam_rot[1] -= 0.35f;

        if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_UP])
            cam_rot[0] += 0.35f;
        else if (buttons_held[SDL_CONTROLLER_BUTTON_DPAD_DOWN])
            cam_rot[0] -= 0.35f;

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

        glBindTexture(GL_TEXTURE_2D, tex);

        /* draw the same data 3 using different ways to show that they're identical */

        glLoadIdentity();
        glTranslatef(-2.0f, 0.0f, cam_dist);
        glRotatef(cam_rot[0], 1.0f, 0.0f, 0.0f); /* rotate on the X axis */
        glRotatef(cam_rot[1], 0.0f, 1.0f, 0.0f); /* rotate on the Y axis */
        glDrawArrays(GL_TRIANGLES, 0, cube_numverts);

        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, cam_dist);
        glRotatef(cam_rot[0], 1.0f, 0.0f, 0.0f); /* rotate on the X axis */
        glRotatef(cam_rot[1], 0.0f, 1.0f, 0.0f); /* rotate on the Y axis */
        glDrawElements(GL_TRIANGLES, cube_numverts, GL_UNSIGNED_SHORT, cube_indices);

        glLoadIdentity();
        glTranslatef(2.0f, 0.0f, cam_dist);
        glRotatef(cam_rot[0], 1.0f, 0.0f, 0.0f); /* rotate on the X axis */
        glRotatef(cam_rot[1], 0.0f, 1.0f, 0.0f); /* rotate on the Y axis */
        draw_immediate(GL_TRIANGLES, cube_numverts, cube_stride, cube_vbuf);

        pbgl_swap_buffers();
    }

wait_then_cleanup:
    Sleep(5000);
    if (gpu_buf) pbgl_free(gpu_buf);
    pbgl_shutdown();
    SDL_Quit();

    return 0;
}
