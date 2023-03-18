#ifndef _GEOMETRY_RECTANGLE_HPP_
#define _GEOMETRY_RECTANGLE_HPP_
#include "point.hpp"

namespace Geometry {
    struct Rectangle {
        Point<double> bottomLeft, topRight;
        Rectangle(const Point<double> & t_botleft, const Point<double> & t_topRight)
            : bottomLeft { t_botleft }, topRight { t_topRight } {}
        Rectangle(const Rectangle &) = default;
        Rectangle(Rectangle &&) = default;
        ~Rectangle() = default;

        Rectangle & operator=(const Rectangle &) = default;
        Rectangle & operator=(Rectangle &&) = default;
    };
} // namespace Geometry

#endif
