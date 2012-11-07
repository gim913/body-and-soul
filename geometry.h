#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

struct point
{
 int x;
 int y;
 point(int X = 0, int Y = 0) : x (X), y (Y) {}
 point(const point &p) : x (p.x), y (p.y) {}
 ~point(){}
};

struct pointcomp
{
 bool operator() (const point &lhs, const point &rhs) const
 {
  if (lhs.x < rhs.x) return true;
  if (lhs.x > rhs.x) return false;
  if (lhs.y < rhs.y) return true;
  if (lhs.y > rhs.y) return false;
  return false;
 };
};

#endif
