#ifndef _NUMERIC_RUNGE_KUTTA_HPP_
#define _NUMERIC_RUNGE_KUTTA_HPP_
#include "cartesian_grid_of_speed.hpp"
#include "cloud_of_points.hpp"
#include "vortex.hpp"

#include <utility>

namespace Numeric {

    Geometry::CloudOfPoints solve_RK4_fixed_vortices(double dt,
                                                     const CartesianGridOfSpeed & speed,
                                                     const Geometry::CloudOfPoints & t_points);

    Geometry::CloudOfPoints solve_RK4_movable_vortices(double dt,
                                                       CartesianGridOfSpeed & t_velocity,
                                                       Simulation::Vortices & t_vortices,
                                                       const Geometry::CloudOfPoints & t_points);
} // namespace Numeric

#endif
