#ifndef EVAS_3D_PRIMITIVES_H
#define EVAS_3D_PRIMITIVES_H

#include <Eo.h>
#include <Evas.h>
#include <math.h>

// TODO Use an external library of linear algebra.
typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

typedef struct _vec2
{
    float   x;
    float   y;
} vec2;

/* The type of user-defined parametric surface function.*/
typedef vec3 (Surface)(float x, float y);

/* Set frame as sphere. */
void
evas_3d_add_sphere_frame(Eo *mesh, int frame, int precision, vec2 tex_scale);

/* Set frame as user defined parametric surface.A parametric surface is a
 * surface in the Euclidean space R3 which is defined by a parametric equation
 * with two parameters. */
void
evas_3d_add_func_surface_frame(Eo *mesh, int frame, Surface func, int precision, vec2 tex_scale);

/* Set frame as sphere as torus */
void
evas_3d_add_torus_frame(Eo *mesh, int frame, float rratio, int precision, vec2 tex_scale);

/* Set frame as cylinder. */
void
evas_3d_add_cylinder_frame(Eo *mesh, int frame, int precision, vec2 tex_scale);

/* Set frame as cone. */
void
evas_3d_add_cone_frame(Eo *mesh, int frame, int precision, vec2 tex_scale);

/* Set frame as square. */
void
evas_3d_add_square_frame(Eo *mesh, int frame);

/* Set frame as cube. */
void
evas_3d_add_cube_frame(Eo *mesh, int frame);

#endif // EVAS_3D_PRIMITIVES_H
