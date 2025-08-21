#include "vector.h"
#include <math.h>
#include <string.h>

/* Forward decls for your backend (provided by graphics.c). */
void draw_line(int x1, int y1, int x2, int y2, unsigned char c);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned char c, bool filled);
void draw_circle(int x, int y, int r, unsigned char c, bool filled);
void draw_rect(int x1, int y1, int x2, int y2, unsigned char c, bool filled);

/* ----------------------- Small internal utilities ------------------------ */
static inline int iabs(int v) {
  return v < 0 ? -v : v;
}
static inline int isq(int v) {
  return v * v;
}
static inline int max3(int a, int b, int c) {
  return (a > b ? (a > c ? a : c) : (b > c ? b : c));
}
static inline int min3(int a, int b, int c) {
  return (a < b ? (a < c ? a : c) : (b < c ? b : c));
}

void vg_path_init(VGPath *p, VGCmd *cmd_buf, int cmd_cap, VGPoint *pt_buf, int pt_cap) {
  p->cmds      = cmd_buf;
  p->pts       = pt_buf;
  p->cmd_cap   = cmd_cap;
  p->pt_cap    = pt_cap;
  p->cmd_count = sizeof(cmd_buf);
  p->pt_count  = sizeof(pt_buf);
}

void vg_path_reset(VGPath *p) {
  p->cmd_count = 0;
  p->pt_count  = 0;
}

static bool vg_push_cmd(VGPath *p, VGCmd c) {
  if (p->cmd_count >= p->cmd_cap) return false;
  p->cmds[p->cmd_count++] = c;
  return true;
}
static bool vg_push_pt(VGPath *p, int x, int y) {
  if (p->pt_count >= p->pt_cap) return false;
  p->pts[p->pt_count++] = (VGPoint){x, y};
  return true;
}

bool vg_move_to(VGPath *p, int x, int y) {
  return vg_push_cmd(p, VG_MOVE_TO) && vg_push_pt(p, x, y);
}
bool vg_line_to(VGPath *p, int x, int y) {
  return vg_push_cmd(p, VG_LINE_TO) && vg_push_pt(p, x, y);
}
bool vg_quad_to(VGPath *p, int cx, int cy, int x, int y) {
  return vg_push_cmd(p, VG_QUAD_TO) && vg_push_pt(p, cx, cy) && vg_push_pt(p, x, y);
}
bool vg_cubic_to(VGPath *p, int c1x, int c1y, int c2x, int c2y, int x, int y) {
  return vg_push_cmd(p, VG_CUBIC_TO) && vg_push_pt(p, c1x, c1y) && vg_push_pt(p, c2x, c2y) && vg_push_pt(p, x, y);
}
bool vg_close(VGPath *p) {
  return vg_push_cmd(p, VG_CLOSE);
}

/* ------------------------- Curve flattening ------------------------------ */
/* Adaptive subdivision for quadratic and cubic Bézier curves. */

typedef struct {
  int x, y;
} I2;
static inline I2 i2(int x, int y) {
  I2 r = {x, y};
  return r;
}
static inline I2 i2_mid(I2 a, I2 b) {
  return i2((a.x + b.x) >> 1, (a.y + b.y) >> 1);
}
static inline int dist_pt_seg_sq(I2 p, I2 a, I2 b) {
  int vx = b.x - a.x, vy = b.y - a.y;
  int wx = p.x - a.x, wy = p.y - a.y;
  int c1 = vx * wx + vy * wy;
  if (c1 <= 0) return isq(p.x - a.x) + isq(p.y - a.y);
  int c2 = vx * vx + vy * vy;
  if (c2 <= c1) return isq(p.x - b.x) + isq(p.y - b.y);
  // projection t = c1/c2 (float), approximate with integer rounding
  // For tolerance purposes, we can do fixed-point (16.16) but keep it simple here
  double t  = (double)c1 / (double)c2;
  double px = a.x + t * vx, py = a.y + t * vy;
  int    dx = (int)llround(px) - p.x;
  int    dy = (int)llround(py) - p.y;
  return dx * dx + dy * dy;
}

static void flatten_quad(I2 p0, I2 p1, I2 p2, int flat_sq, int subdiv_limit, VGPoint *out, int *count, int maxn) {
  // Stack-based subdivisions
  typedef struct {
    I2  a, b, c;
    int level;
  } Q;
  Q   stack[64];
  int sp      = 0;
  stack[sp++] = (Q){p0, p1, p2, 0};
  I2 last     = p0; // emit start once (caller will manage joins)
  while (sp) {
    Q q = stack[--sp];
    // Compute the deviation of control point from baseline
    int d = dist_pt_seg_sq(q.b, q.a, q.c);
    if (d <= flat_sq || q.level >= subdiv_limit - 1) {
      if (*count < maxn) out[(*count)++] = (VGPoint){q.c.x, q.c.y};
      last = q.c;
      continue;
    }
    // Subdivide via de Casteljau
    I2 a = q.a, b = q.b, c = q.c;
    I2 ab = i2_mid(a, b), bc = i2_mid(b, c);
    I2 abc      = i2_mid(ab, bc);
    stack[sp++] = (Q){abc, bc, c, q.level + 1};
    stack[sp++] = (Q){a, ab, abc, q.level + 1};
  }
}

static void flatten_cubic(I2 p0, I2 p1, I2 p2, I2 p3, int flat_sq, int subdiv_limit, VGPoint *out, int *count, int maxn) {
  typedef struct {
    I2  a, b, c, d;
    int level;
  } C;
  C   stack[64];
  int sp      = 0;
  stack[sp++] = (C){p0, p1, p2, p3, 0};
  while (sp) {
    C cu = stack[--sp];
    // Use the "convex hull" distance heuristic (max of distances of inner control points to baseline)
    int d1 = dist_pt_seg_sq(cu.b, cu.a, cu.d);
    int d2 = dist_pt_seg_sq(cu.c, cu.a, cu.d);
    int d  = d1 > d2 ? d1 : d2;
    if (d <= flat_sq || cu.level >= subdiv_limit - 1) {
      if (*count < maxn) out[(*count)++] = (VGPoint){cu.d.x, cu.d.y};
      continue;
    }
    // de Casteljau split
    I2 a = cu.a, b = cu.b, c = cu.c, dpt = cu.d;
    I2 ab = i2_mid(a, b), bc = i2_mid(b, c), cd = i2_mid(c, dpt);
    I2 abbc = i2_mid(ab, bc), bccd = i2_mid(bc, cd);
    I2 mid      = i2_mid(abbc, bccd);
    stack[sp++] = (C){mid, bccd, cd, dpt, cu.level + 1};
    stack[sp++] = (C){a, ab, abbc, mid, cu.level + 1};
  }
}

/* ---------------------- Polyline stroke/fill core ------------------------ */

void vg_draw_polyline(const VGPoint *pts, int n, unsigned char color) {
  if (n < 2) return;
  for (int i = 0; i < n - 1; i++) {
    draw_line(pts[i].x, pts[i].y, pts[i + 1].x, pts[i + 1].y, color);
  }
}

/* Ear clipping triangulation for simple (non-self-intersecting) polygons. */
static int cross_z(VGPoint a, VGPoint b, VGPoint c) {
  long ax = b.x - a.x, ay = b.y - a.y;
  long bx = c.x - b.x, by = c.y - b.y;
  long z = ax * by - ay * bx;
  return (z > 0) - (z < 0);
}
static bool point_in_tri(VGPoint p, VGPoint a, VGPoint b, VGPoint c) {
  long ab      = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
  long bc      = (c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x);
  long ca      = (a.x - c.x) * (p.y - c.y) - (a.y - c.y) * (p.x - c.x);
  bool has_neg = (ab < 0) || (bc < 0) || (ca < 0);
  bool has_pos = (ab > 0) || (bc > 0) || (ca > 0);
  return !(has_neg && has_pos);
}

void vg_fill_polygon(const VGPoint *in_pts, int n, unsigned char color) {
  if (n < 3) return;
  // Make a local index list
  int idx[256];
  if (n > 256) n = 256; // safety cap for embedded
  for (int i = 0; i < n; i++)
    idx[i] = i;

  int remaining = n;
  int guard     = 0; // guard prevents infinite loops on bad polygons
  while (remaining > 2 && guard < 10000) {
    guard++;
    bool ear_found = false;
    for (int i = 0; i < remaining; i++) {
      int     i0 = idx[(i + remaining - 1) % remaining];
      int     i1 = idx[i];
      int     i2 = idx[(i + 1) % remaining];
      VGPoint a = in_pts[i0], b = in_pts[i1], c = in_pts[i2];
      // Check convexity (assume CCW fill). If CW, flip once below.
      long z = (long)(b.x - a.x) * (c.y - a.y) - (long)(b.y - a.y) * (c.x - a.x);
      if (z <= 0) continue; // reflex vertex or collinear
      bool contains = false;
      for (int j = 0; j < remaining; j++)
        if (j != i && j != (i + 1) % remaining && j != (i + remaining - 1) % remaining) {
          int k = idx[j];
          if (point_in_tri(in_pts[k], a, b, c)) {
            contains = true;
            break;
          }
        }
      if (contains) continue;
      // Clip ear
      draw_triangle(a.x, a.y, b.x, b.y, c.x, c.y, color, true);
      // Remove vertex i1
      for (int m = i; m < remaining - 1; m++)
        idx[m] = idx[m + 1];
      remaining--;
      ear_found = true;
      break;
    }
    if (!ear_found) {
      // Likely CW; try a single reversal then fallback to bail out
      for (int l = 0; l < remaining / 2; l++) {
        int t                  = idx[l];
        idx[l]                 = idx[remaining - 1 - l];
        idx[remaining - 1 - l] = t;
      }
    }
  }
}

/* -------------------------- Path rendering ------------------------------- */

static VGPoint tmp_pts[1024];

static int     flatten_path_to_polyline(const VGPath *p, VGPoint *poly, int maxn, const VGFlatten *flt) {
  int  outn      = 0;
  I2   cur       = {0, 0};
  I2   start     = {0, 0};
  bool has_start = false;
  int  flat_sq   = flt ? (flt->curve_flatness_sq > 0 ? flt->curve_flatness_sq : 1) : 1;
  int  limit     = flt ? (flt->subdiv_limit > 0 ? flt->subdiv_limit : 12) : 12;

  int  ip        = 0; // index into p->pts
  for (int ic = 0; ic < p->cmd_count; ic++) {
    VGCmd c = p->cmds[ic];
    switch (c) {
    case VG_MOVE_TO: {
      if (has_start && outn < maxn) { /* implicitly move creates new subpath: insert separator via duplicate? */
      }
      cur = (I2){p->pts[ip].x, p->pts[ip].y};
      ip++;
      start     = cur;
      has_start = true;
      if (outn < maxn) poly[outn++] = (VGPoint){cur.x, cur.y};
    } break;

    case VG_LINE_TO: {
      I2 p1 = (I2){p->pts[ip].x, p->pts[ip].y};
      ip++;
      if (outn < maxn) poly[outn++] = (VGPoint){p1.x, p1.y};
      cur = p1;
    } break;

    case VG_QUAD_TO: {
      I2 c1 = (I2){p->pts[ip].x, p->pts[ip].y};
      I2 p1 = (I2){p->pts[ip + 1].x, p->pts[ip + 1].y};
      ip += 2;
      int base = outn;
      if (outn < maxn) poly[outn++] = (VGPoint){cur.x, cur.y};
      int cnt = 0;
      flatten_quad(cur, c1, p1, flat_sq, limit, tmp_pts, &cnt, (int)(sizeof(tmp_pts) / sizeof(tmp_pts[0])));
      for (int i = 0; i < cnt && outn < maxn; i++)
        poly[outn++] = tmp_pts[i];
      cur = p1;
    } break;

    case VG_CUBIC_TO: {
      I2 c1 = (I2){p->pts[ip].x, p->pts[ip].y};
      I2 c2 = (I2){p->pts[ip + 1].x, p->pts[ip + 1].y};
      I2 p1 = (I2){p->pts[ip + 2].x, p->pts[ip + 2].y};
      ip += 3;
      int base = outn;
      if (outn < maxn) poly[outn++] = (VGPoint){cur.x, cur.y};
      int cnt = 0;
      flatten_cubic(cur, c1, c2, p1, flat_sq, limit, tmp_pts, &cnt, (int)(sizeof(tmp_pts) / sizeof(tmp_pts[0])));
      for (int i = 0; i < cnt && outn < maxn; i++)
        poly[outn++] = tmp_pts[i];
      cur = p1;
    } break;

    case VG_CLOSE: {
      if (has_start) {
        if (outn < maxn) poly[outn++] = (VGPoint){start.x, start.y};
        cur = start;
      }
    } break;
    }
  }
  return outn;
}

void vg_stroke_path(const VGPath *p, unsigned char color, const VGStrokeStyle *style, const VGFlatten *flt) {
  VGStrokeStyle s = style ? *style : (VGStrokeStyle){1, VG_CAP_BUTT, VG_JOIN_MITER, 4};

  // For simplicity and speed on embedded, we handle width==1 by drawing the centerline polyline.
  // Wider strokes can be approximated by expanding to a triangle strip (butt caps) — here we keep it minimal.
  static VGPoint poly[1024];
  int            n = flatten_path_to_polyline(p, poly, 1024, flt);
  if (s.width <= 1) {
    vg_draw_polyline(poly, n, color);
    return;
  }

  // Very simple "fat line": draw parallel offsets using Manhattan approximation
  int half = s.width / 2;
  for (int off = -half; off <= half; off++) {
    // Offset in 4-neighbour directions per segment to approximate thickness
    for (int i = 0; i < n - 1; i++) {
      VGPoint a = poly[i], b = poly[i + 1];
      if (iabs(b.x - a.x) >= iabs(b.y - a.y)) {
        draw_line(a.x, a.y + off, b.x, b.y + off, color);
      } else {
        draw_line(a.x + off, a.y, b.x + off, b.y, color);
      }
    }
  }
}

void vg_fill_path(const VGPath *p, unsigned char color, const VGFlatten *flt) {
  static VGPoint poly[1024];
  int            n = flatten_path_to_polyline(p, poly, 1024, flt);
  if (n >= 3) vg_fill_polygon(poly, n, color);
}
