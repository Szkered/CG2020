/*!
 *      \file Circle.h
 *      \brief Planar circle
 *	   \author David Gu
 *      \date 10/19/2010
 *
 */

#ifndef _MESHLIB_CIRCLE_H_
#define _MESHLIB_CIRCLE_H_

#include "Line2D.h"

namespace MeshLib
{

/*!
 *	\brief CCircle class, circle on the plane
 *
 *	Circle on the two dimensional plane
 */
class CCircle
{
  public:
    /*!
     *	CCircle default constructor, it is initialized to be ((0,0),1)
     */
    CCircle()
    {
        m_c = CPoint2(0, 0);
        m_r = 1.0;
    };
    /*!
     *	CCircle class copy operator
     */
    CCircle(const CPoint2 &c, const double r)
    {
        m_c = c;
        m_r = r;
    };
    /*!
     *	CCircle class destructor
     */
    ~CCircle(){};
    /*!
     *	Three points determines a circle
     */
    CCircle(CPoint2 p[3])
    {
        double x[3][3];

        for (int i = 0; i < 3; i++)
        {
            x[i][0] = p[i][0];
            x[i][1] = p[i][1];
            x[i][2] = 1.0;
        }

        double a = det(x);

        for (int i = 0; i < 3; i++)
        {
            x[i][0] = p[i][0] * p[i][0] + p[i][1] * p[i][1];
            x[i][1] = p[i][1];
            x[i][2] = 1.0;
        }

        double d = -det(x);

        for (int i = 0; i < 3; i++)
        {
            x[i][0] = p[i][0] * p[i][0] + p[i][1] * p[i][1];
            x[i][1] = p[i][0];
            x[i][2] = 1.0;
        }

        double e = det(x);

        for (int i = 0; i < 3; i++)
        {
            x[i][0] = p[i][0] * p[i][0] + p[i][1] * p[i][1];
            x[i][1] = p[i][0];
            x[i][2] = p[i][1];
        }

        double f = -det(x);

        m_c = CPoint2(-d / (2 * a), -e / (2 * a));
        m_r = sqrt((d * d + e * e) / (4.0 * a * a) - f / a);
    }

    /*! The center of the circle
     *
     */
    CPoint2 &c()
    {
        return m_c;
    };

    /*! The radius of the circle
     *
     */
    double &r()
    {
        return m_r;
    };

  private:
    /*!
     * Center
     */
    CPoint2 m_c;
    /*!
     * radius
     */
    double m_r;

  protected:
    // determinant of a 3x3 matrix
    double det(double x[3][3])
    {
        double d = x[0][0] * x[1][1] * x[2][2] + x[0][2] * x[1][0] * x[2][1] + x[0][1] * x[1][2] * x[2][0] -
                   x[2][0] * x[1][1] * x[0][2] - x[2][1] * x[1][2] * x[0][0] - x[2][2] * x[1][0] * x[0][1];
        return d;
    };
};

/*!
 *	Computing a circle orthogonal to three other circles
 */
inline CCircle orthogonal(CCircle C[3])
{

    // The cricle C_i is : <p,p> - 2<p,c_i> = d_i
    double d[3];
    for (int i = 0; i < 3; i++)
    {
        d[i] = C[i].r() * C[i].r() - mag2(C[i].c());
    }

    // the common chord lines are <p,dc_k> = h_k
    CPoint2 dc[2];

    dc[0] = C[1].c() - C[0].c();
    dc[1] = C[2].c() - C[1].c();

    double h[2];
    h[0] = (d[0] - d[1]) / 2.0;
    h[1] = (d[1] - d[2]) / 2.0;

    double m[2][2];

    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
        {
            m[i][j] = dc[i] * dc[j];
        }

    double det = m[0][0] * m[1][1] - m[1][0] * m[0][1];
    double inv[2][2];

    inv[0][0] = m[1][1] / det;
    inv[1][1] = m[0][0] / det;
    inv[0][1] = -m[0][1] / det;
    inv[1][0] = -m[1][0] / det;

    double x[2];
    x[0] = inv[0][0] * h[0] + inv[0][1] * h[1];
    x[1] = inv[1][0] * h[0] + inv[1][1] * h[1];

    CPoint2 center = dc[0] * x[0] + dc[1] * x[1];

    CPoint2 diff = center - C[0].c();
    double radius = sqrt(mag2(diff) - C[0].r() * C[0].r());

    // printf("%f %f %f\n", radius, sqrt( mag2(center-C[1].c()) - C[1].r()*C[1].r() ), sqrt( mag2(center-C[2].c()) -
    // C[2].r()*C[2].r() ) );

    return CCircle(center, radius);
};

inline int _circle_circle_intersection(CCircle C0, CCircle C1, CPoint2 &p0, CPoint2 &p1)
{

    double x0 = C0.c()[0];
    double y0 = C0.c()[1];
    double r0 = C0.r();

    double x1 = C1.c()[0];
    double y1 = C1.c()[1];
    double r1 = C1.r();

    double a, dx, dy, d, h, rx, ry;
    double x2, y2;

    /* dx and dy are the vertical and horizontal distances between
     * the circle centers.
     */
    dx = x1 - x0;
    dy = y1 - y0;

    /* Determine the straight-line distance between the centers. */
    d = sqrt((dy * dy) + (dx * dx));

    /* Check for solvability. */
    if (d > (r0 + r1))
    {
        /* no solution. circles do not intersect. */
        return 0;
    }
    if (d < abs(r0 - r1))
    {
        /* no solution. one circle is contained in the other */
        return 0;
    }

    /* 'point 2' is the point where the line through the circle
     * intersection points crosses the line between the circle
     * centers.
     */

    /* Determine the distance from point 0 to point 2. */
    a = ((r0 * r0) - (r1 * r1) + (d * d)) / (2.0 * d);

    /* Determine the coordinates of point 2. */
    x2 = x0 + (dx * a / d);
    y2 = y0 + (dy * a / d);

    /* Determine the distance from point 2 to either of the
     * intersection points.
     */
    h = sqrt((r0 * r0) - (a * a));

    /* Now determine the offsets of the intersection points from
     * point 2.
     */
    rx = -dy * (h / d);
    ry = dx * (h / d);

    /* Determine the absolute intersection points. */
    p0 = CPoint2(x2 + rx, y2 + ry);
    p1 = CPoint2(x2 - rx, y2 - ry);

    return 1;
};

inline bool intersect(CCircle &CL, CLine2D &L, CPoint2 &q1, CPoint2 &q2)
{
    CPoint2 n = L.n();
    CPoint2 d(-n[1], n[0]);
    CPoint2 q = n * L.d();
    CPoint2 c = CL.c();
    double r = CL.r();

    double A = d * d;
    double B = d * (q - c) * 2;
    double C = (q - c) * (q - c) - r * r;

    double delta = B * B - 4 * A * C;

    if (delta < 0)
        return false;

    double lambda1 = (-B + sqrt(delta)) / (2 * A);
    double lambda2 = (-B - sqrt(delta)) / (2 * A);

    q1 = q + d * lambda1;
    q2 = q + d * lambda2;

    return true;
};

}; // namespace MeshLib

#endif
