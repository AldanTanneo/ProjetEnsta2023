#ifndef _SIMULATION_Vortices_HPP_
#define _SIMULATION_Vortices_HPP_
#include "point.hpp"
#include "vector.hpp"

#include <cassert>
#include <mpi.h>
#include <vector>

namespace Simulation {
    class Vortices {
    public:
        using container = std::vector<double>;
        using point = Geometry::Point<double>;
        using vector = Geometry::Vector<double>;
        Vortices() = default;
        Vortices(std::size_t nbVortices, const std::pair<point, point> & domain)
            : m_centers_and_intensities(3 * nbVortices),
              m_domainSize(domain.first, domain.second) {}
        Vortices(const Vortices &) = default;
        Vortices(Vortices &&) = default;
        ~Vortices() = default;

        std::size_t numberOfVortices() const { return m_centers_and_intensities.size() / 3; }

        point getCenter(std::size_t t_index) const {
            assert(t_index < numberOfVortices());
            return { m_centers_and_intensities[3 * t_index + 0],
                     m_centers_and_intensities[3 * t_index + 1] };
        }

        double getIntensity(std::size_t t_index) const {
            assert(t_index < numberOfVortices());
            return m_centers_and_intensities[3 * t_index + 2];
        }

        void setVortex(std::size_t t_index, const point & t_center, double t_intensity) {
            assert(t_index < numberOfVortices());
            assert(t_intensity != 0);
            m_centers_and_intensities[3 * t_index + 0] = t_center.x;
            m_centers_and_intensities[3 * t_index + 1] = t_center.y;
            m_centers_and_intensities[3 * t_index + 2] = t_intensity;
        }

        void removeVortex(std::size_t t_index) {
            assert(t_index < numberOfVortices());
            std::size_t lastIndex = numberOfVortices() - 1;
            m_centers_and_intensities[3 * t_index + 0] =
                m_centers_and_intensities[3 * lastIndex + 0];
            m_centers_and_intensities[3 * t_index + 1] =
                m_centers_and_intensities[3 * lastIndex + 1];
            m_centers_and_intensities[3 * t_index + 2] =
                m_centers_and_intensities[3 * lastIndex + 2];
            m_centers_and_intensities.resize(numberOfVortices() - 3);
        }

        void addNewVortex(const point & t_center, double t_intensity) {
            std::size_t lastIndex = numberOfVortices();
            m_centers_and_intensities.resize(3 * lastIndex + 3);
            m_centers_and_intensities[3 * lastIndex + 0] = t_center.x;
            m_centers_and_intensities[3 * lastIndex + 1] = t_center.y;
            m_centers_and_intensities[3 * lastIndex + 2] = t_intensity;
        }

        vector computeSpeed(const point & a_point) const;

        Vortices & operator=(const Vortices &) = default;
        Vortices & operator=(Vortices &&) = default;

        constexpr static int TAG = 'V';

        inline int send(int dest, MPI_Comm comm) const {
            return MPI_Send(m_centers_and_intensities.data(), m_centers_and_intensities.size(),
                            MPI_DOUBLE, dest, Vortices::TAG, comm);
        }

        inline int recv(int source, MPI_Comm comm, MPI_Status * status) {
            return MPI_Recv(m_centers_and_intensities.data(), m_centers_and_intensities.size(),
                            MPI_DOUBLE, source, Vortices::TAG, comm, status);
        }

    private:
        container m_centers_and_intensities;
        vector m_domainSize;
    };
} // namespace Simulation

#endif
