#ifndef _GRAPHISM_SCREEN_HPP_
#define _GRAPHISM_SCREEN_HPP_
#include "cartesian_grid_of_speed.hpp"
#include "cloud_of_points.hpp"
#include "vortex.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Window.hpp>
#include <utility>

namespace Graphisme {
    class Screen {
    public:
        Screen(const std::pair<std::size_t, std::size_t> & t_geometry,
               const std::pair<Geometry::Point<double>, Geometry::Point<double>> & t_domain);

        bool isOpen() const { return m_window.isOpen(); }

        bool pollEvent(sf::Event & t_event) { return m_window.pollEvent(t_event); }

        void close() { m_window.close(); }

        void displayVelocityField(const Numeric::CartesianGridOfSpeed & grid,
                                  const Simulation::Vortices & vortices);
        void displayParticles(const Numeric::CartesianGridOfSpeed & grid,
                              const Simulation::Vortices & vortices,
                              const Geometry::CloudOfPoints & points);

        void clear(sf::Color t_color) { m_window.clear(t_color); }

        void resize(sf::Event & event) {
            sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
            m_window.setView(sf::View(visibleArea));
        }

        std::pair<std::size_t, std::size_t> getGeometry() const {
            return { m_window.getSize().x, m_window.getSize().y };
        }

        void drawText(const std::string & text, const Geometry::Point<double> & position);

        void display() { m_window.display(); }

    private:
        sf::RenderWindow m_window;
        sf::Font m_font;
        sf::View m_velocityView, m_particlesView;
        sf::VertexArray m_grid;      /// Grid display
        sf::VertexArray m_velocity;  /// Velocity display
        sf::VertexArray m_particles; /// Particles display
    };
} // namespace Graphisme

#endif
