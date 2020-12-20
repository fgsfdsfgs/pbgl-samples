#ifndef _PBGL_SAMPLE_OBJECT_H
#define _PBGL_SAMPLE_OBJECT_H

#include <GL/gl.h>

#define MAX_POINTS 256
#define MAX_PLANES 128

// vertex in 3d-coordinate system
typedef struct {
    float x, y, z;
} Point;

// plane equation
typedef struct {
    float a, b, c, d;
} PlaneEq;

// structure describing an object's face
typedef struct {
    GLuint p[3];
    Point normals[3];
    GLuint neigh[3];
    PlaneEq planeEq;
    GLboolean visible;
} Plane;

// object structure
typedef struct {
    GLuint nPlanes, nPoints;
    Point points[MAX_POINTS];
    Plane planes[MAX_PLANES];
} GLObject;

GLboolean globj_read(GLObject *o, const char *fname);
void globj_set_connectivity(GLObject *o);
void globj_calc_plane(GLObject *o, Plane *plane);
void globj_draw(const GLObject *o);
void globj_cast_shadow(GLObject *o, const float *lightpos);

#endif // _PBGL_SAMPLE_OBJECT_H
