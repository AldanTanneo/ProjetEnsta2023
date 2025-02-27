#ifndef _NUMERICAL_CARTESIAN_GRID_OF_SPEED_HPP_
#define _NUMERICAL_CARTESIAN_GRID_OF_SPEED_HPP_
#include "point.hpp"
#include "vector.hpp"
#include "vortex.hpp"

#include <mpi.h>
#include <utility>
#include <vector>

namespace Numeric {
    /**
     * @brief Cartesian grid containing velocity field computing from vortices
     *
     */
    class CartesianGridOfSpeed {
    public:
        using vector = Geometry::Vector<double>;
        using container = std::vector<vector>;
        using point = Geometry::Point<double>;

        //@name Constructors and destructor
        //@{
        /**
         * @brief Default constructor.
         *
         * Build a undimensionned and uninitialized velocity field grid.
         *
         */
        CartesianGridOfSpeed() = default;
        /**
         * @brief Construct a new cartesian grid with uninitialized velocity
         * field
         *
         * @param t_dimensions The dimension of the cartesian grid in number of
         * cells per direction
         * @param m_origin     The origin (left bottom vertex) coordinate of the
         * cartesian grid
         * @param t_hStep      The step in all direction for the regular grid
         */
        CartesianGridOfSpeed(std::pair<std::size_t, std::size_t> t_dimensions,
                             Geometry::Point<double> m_origin,
                             double t_hStep);
        /**
         * @brief Copy constructor
         *
         */
        CartesianGridOfSpeed(const CartesianGridOfSpeed &) = default;
        /**
         * @brief Move constructor
         *
         */
        CartesianGridOfSpeed(CartesianGridOfSpeed &&) = default;
        /**
         * @brief Destructor
         *
         */
        ~CartesianGridOfSpeed() = default;
        //@}

        //@name Accessors and modifiers
        //@{
        /**
         * @brief Get the Left Bottom Vertex point
         *
         * @return point The coordinate of the left bottom point
         */
        point getLeftBottomVertex() const { return point { m_left, m_bottom }; }
        /**
         * @brief Get the Right Top Vertex point
         *
         * @return point The coordinate of the right top vertex point
         */
        point getRightTopVertex() const {
            return point { m_left + m_width * m_step, m_bottom + m_height * m_step };
        }
        /**
         * @brief Return the dimension of the cartesian grid in number of cells
         * per direction
         *
         * @return std::pair<std::size_t,std::size_t>
         */
        std::pair<std::size_t, std::size_t> cellGeometry() const { return { m_width, m_height }; }

        /**
         * @brief Return the address of the first velocity vector of the
         * velocityfield
         *
         * @return double* Return as a double value.
         */
        double * data() { return (double *)m_velocityField.data(); }
        const double * data() const { return (const double *)m_velocityField.data(); }

        double getStep() const { return m_step; }

        void updateVelocityField(const Simulation::Vortices & t_vortices);

        vector getVelocity(std::size_t iCell, std::size_t jCell) const {
            return m_velocityField[iCell * m_width + jCell];
        }

        point updatePosition(const point & pt) const;

        vector computeVelocityFor(const point & p) const;

        CartesianGridOfSpeed & operator=(const CartesianGridOfSpeed &) = default;
        CartesianGridOfSpeed & operator=(CartesianGridOfSpeed &&) = default;

        constexpr static int TAG = 'G';

        inline int send(int dest, MPI_Comm comm) const {
            return MPI_Send(data(), (sizeof(vector) / sizeof(double)) * m_velocityField.size(),
                            MPI_DOUBLE, dest, CartesianGridOfSpeed::TAG, comm);
        }

        inline int recv(int source, MPI_Comm comm, MPI_Status * status) {
            return MPI_Recv(data(), (sizeof(vector) / sizeof(double)) * m_velocityField.size(),
                            MPI_DOUBLE, source, CartesianGridOfSpeed::TAG, comm, status);
        }

    private:
        std::size_t m_width, m_height;
        double m_left, m_bottom;
        double m_step;
        container m_velocityField;
    };
} // namespace Numeric

#endif
