#pragma once

//#include "utility.h"

namespace night
{

//#include <bits/stdc++.h>
//using namespace std;

//// Stores the X and Y coordinate of
//// a point respectively
//#define vec2 pair<real, real>

// Function to find the line given
// two points
void lineFromPoints(vec2 P, vec2 Q, real& a,
    real& b, real& c)
{
    a = Q.y - P.y;
    b = P.x - Q.x;
    c = a * (P.x) + b * (P.y);
}

// Function to convert the input line
// to its perpendicular bisector
void perpendicularBisector(
    vec2 P, vec2 Q, real& a,
    real& b, real& c)
{
    vec2 mid_point = { (P.x + Q.x) / 2,
                      (P.y + Q.y) / 2 };

    // c = -bx + ay
    c = -b * (mid_point.x)
        + a * (mid_point.y);

    real temp = a;
    a = -b;
    b = temp;
}

// Function to find the
// intersection point of two lines
vec2 lineLineIntersection(
    real a1, real b1,
    real c1, real a2,
    real b2, real c2)
{
    real determinant = a1 * b2 - a2 * b1;

    // As points are non-collinear,
    // determinant cannot be 0
    real x = (b2 * c1 - b1 * c2)
        / determinant;
    real y = (a1 * c2 - a2 * c1)
        / determinant;

    return vec2(x, y);
}

// Function to find the
// circumcenter of a triangle
vec2 findCircumCenter(vec2 A[])
{
    vec2 P, Q, R;
    P = A[0], Q = A[1], R = A[2];

    // Line PQ is represented as
    // ax + by = c
    real a, b, c;
    lineFromPoints(P, Q, a, b, c);

    // Line QR is represented as
    // ex + fy = g
    real e, f, g;
    lineFromPoints(Q, R, e, f, g);

    // Converting lines PQ and QR
    // to perpendicular bisectors
    perpendicularBisector(P, Q, a, b, c);
    perpendicularBisector(Q, R, e, f, g);

    // Their point of intersection
    // gives the circumcenter
    vec2 circumcenter
        = lineLineIntersection(a, b, c,
            e, f, g);

    // Return the circumcenter
    return circumcenter;
}

// Function to find the
// centroid of a triangle
vec2 findCentroid(vec2 A[])
{
    // Centroid of a triangle is
    // given as (Xa + Xb + Xc)/3,
    // (Ya + Yb + Yc)/3
    vec2 centroid
        = { (A[0].x + A[1].x
             + A[2].x)
                / 3,
            (A[0].y + A[1].y
             + A[2].y)
                / 3 };

    // Return the centroid
    return centroid;
}

// TODO: implement this.
vec2 calculate_orthocenter(const vec2& p1, const vec2& p2, const vec2& p3)
{
    vec2 A[3] = {p1, p2, p3};
    vec2 circumcenter = findCircumCenter(A);
    vec2 centroid = findCentroid(A);

    vec2 h = { (3 * centroid.x
               - 2 * circumcenter.x),
              (3 * centroid.y
               - 2 * circumcenter.y) };

    return h;
}

}