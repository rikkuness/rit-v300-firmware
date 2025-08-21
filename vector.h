/*
 * Embedded Vector Graphics Layer
 * Minimal, efficient path rendering on top of your graphics backend.
 * (Written by ChatGPT so it's probably shit)
 *
 * Backend requirements (from graphics.c):
 *   void draw_line(int x1,int y1,int x2,int y2,unsigned char c);
 *   void draw_triangle(int x1,int y1,int x2,int y2,int x3,int y3,unsigned char
 * c,bool filled);
 *   // Optionally used if you want circle/rect helpers:
 *   void draw_circle(int x,int y,int r,unsigned char c,bool filled);
 *   void draw_rect(int x1,int y1,int x2,int y2,unsigned char c,bool filled);
 *
 * This file contains both vector.h and vector.c for convenience.
 */

/********************************** vector.h **********************************/
#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

/* Integer point type (fixed pixel coords). */
typedef struct {
  int x, y;
} VGPoint;

/* Path command stream */
typedef enum {
  VG_MOVE_TO = 0,
  VG_LINE_TO = 1,
  VG_QUAD_TO = 2,  // uses 2 points: control, end
  VG_CUBIC_TO = 3, // uses 3 points: c1, c2, end
  VG_CLOSE = 4
} VGCmd;

/* Lightweight path object using caller-provided buffers (no malloc). */
typedef struct {
  VGCmd *cmds;  // capacity: cmd_cap
  VGPoint *pts; // capacity: pt_cap/
  int cmd_count, pt_count;
  int cmd_cap, pt_cap;
} VGPath;

/* Stroke style (kept intentionally simple). */
typedef enum { VG_CAP_BUTT = 0, VG_CAP_SQUARE = 1, VG_CAP_ROUND = 2 } VGCap;
typedef enum { VG_JOIN_MITER = 0, VG_JOIN_BEVEL = 1, VG_JOIN_ROUND = 2 } VGJoin;

typedef struct {
  int width; // stroke width in pixels (>=1)
  VGCap cap;
  VGJoin join;
  int miter_limit; // in pixels; only for miter joins
} VGStrokeStyle;

/* Tolerances for curve flattening (integer, pixels squared). */
typedef struct {
  int curve_flatness_sq; // default: 1 => ~1px
  int subdiv_limit;      // safety cap to prevent infinite subdivision
} VGFlatten;

/* API */
void vg_path_init(VGPath *p, VGCmd *cmd_buf, int cmd_cap, VGPoint *pt_buf,
                  int pt_cap);
void vg_path_reset(VGPath *p);

bool vg_move_to(VGPath *p, int x, int y);
bool vg_line_to(VGPath *p, int x, int y);
bool vg_quad_to(VGPath *p, int cx, int cy, int x, int y);
bool vg_cubic_to(VGPath *p, int c1x, int c1y, int c2x, int c2y, int x, int y);
bool vg_close(VGPath *p);

/* Rendering */
void vg_stroke_path(const VGPath *p, unsigned char color,
                    const VGStrokeStyle *style, const VGFlatten *flt);
void vg_fill_path(const VGPath *p, unsigned char color, const VGFlatten *flt);

/* Convenience helpers */
void vg_draw_polyline(const VGPoint *pts, int n, unsigned char color);
void vg_fill_polygon(const VGPoint *pts, int n, unsigned char color);

#endif /* VECTOR_H */
