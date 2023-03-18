#include "cartesian_grid_of_speed.hpp"
#include "cloud_of_points.hpp"
#include "runge_kutta.hpp"
#include "screen.hpp"
#include "ui_events.hpp"
#include "vortex.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <string>
#include <tuple>

constexpr int SCREEN_PROCESS = 0;
constexpr int SIM_PROCESS = 1;

static char errstr[MPI_MAX_ERROR_STRING + 1] = "no error";
static int errlen = 0;

#define DISABLE_DEBUGGING // can be used to disable "DEBUG" statements
#include "utils.hpp"

auto readConfigFile(std::ifstream & input) {
    using point = Simulation::Vortices::point;

    int isMobile;
    std::size_t nbVortices;
    Numeric::CartesianGridOfSpeed cartesianGrid;
    Geometry::CloudOfPoints cloudOfPoints;
    constexpr std::size_t maxBuffer = 8192;
    char buffer[maxBuffer];
    std::string sbuffer;
    std::stringstream ibuffer;
    // Lit la première ligne de commentaire :
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lecture de la grille cartésienne
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    double xleft, ybot, h;
    std::size_t nx, ny;
    ibuffer >> xleft >> ybot >> nx >> ny >> h;
    cartesianGrid = Numeric::CartesianGridOfSpeed({ nx, ny }, point { xleft, ybot }, h);
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit mode de génération des particules
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    int modeGeneration;
    ibuffer >> modeGeneration;
    if (modeGeneration == 0) // Génération sur toute la grille
    {
        std::size_t nbPoints;
        ibuffer >> nbPoints;
        cloudOfPoints = Geometry::generatePointsIn(
            nbPoints, { cartesianGrid.getLeftBottomVertex(), cartesianGrid.getRightTopVertex() });
    } else {
        std::size_t nbPoints;
        double xl, xr, yb, yt;
        ibuffer >> xl >> yb >> xr >> yt >> nbPoints;
        cloudOfPoints =
            Geometry::generatePointsIn(nbPoints, { point { xl, yb }, point { xr, yt } });
    }
    // Lit le nombre de vortex :
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit le nombre de vortex
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    try {
        ibuffer >> nbVortices;
    } catch (std::ios_base::failure & err) {
        std::cout << "Error " << err.what() << " found" << std::endl;
        std::cout << "Read line : " << sbuffer << std::endl;
        throw err;
    }
    Simulation::Vortices vortices(
        nbVortices, { cartesianGrid.getLeftBottomVertex(), cartesianGrid.getRightTopVertex() });
    input.getline(buffer, maxBuffer); // Relit un commentaire
    for (std::size_t iVortex = 0; iVortex < nbVortices; ++iVortex) {
        input.getline(buffer, maxBuffer);
        double x, y, force;
        std::string sbuffer(buffer, maxBuffer);
        std::stringstream ibuffer(sbuffer);
        ibuffer >> x >> y >> force;
        vortices.setVortex(iVortex, point { x, y }, force);
    }
    input.getline(buffer, maxBuffer); // Relit un commentaire
    input.getline(buffer, maxBuffer); // Lit le mode de déplacement des vortex
    sbuffer = std::string(buffer, maxBuffer);
    ibuffer = std::stringstream(sbuffer);
    ibuffer >> isMobile;
    return std::make_tuple(vortices, isMobile, cartesianGrid, cloudOfPoints);
}

int main(int argc, char * argv[]) {
    const char * filename;
    if (argc == 1) {
        std::cout << "Usage : vortexsimulator <nom fichier configuration>" << std::endl;
        return EXIT_FAILURE;
    }

    filename = argv[1];
    std::ifstream fich(filename);
    auto config = readConfigFile(fich);
    fich.close();

    std::size_t resx = 800, resy = 600;
    if (argc > 3) {
        resx = std::stoull(argv[2]);
        resy = std::stoull(argv[3]);
    }

    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        return -1;
    }

    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_set_errhandler(comm, MPI_ERRORS_ARE_FATAL);
    int rank = -1, size = -1;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    if (size != 2) {
        if (rank == SCREEN_PROCESS)
            std::cerr << "The program must be launched on two nodes!" << std::endl;
        return -1;
    }

    if (rank == SCREEN_PROCESS) {
        std::cout << "######## Vortex simultor ########" << std::endl << std::endl;
        std::cout << "Press P for play animation " << std::endl;
        std::cout << "Press S to stop animation" << std::endl;
        std::cout << "Press right cursor to advance step by step in time" << std::endl;
        std::cout << "Press down cursor to halve the time step" << std::endl;
        std::cout << "Press up cursor to double the time step" << std::endl;
    }

    auto vortices = std::get<0>(config);
    auto isMobile = std::get<1>(config);
    auto grid = std::get<2>(config);
    auto cloud = std::get<3>(config);

    grid.updateVelocityField(vortices);

    bool animate = false;
    double dt = 0.1;
    bool advance = false;

    UiEvent ui_event = UiEvent::Noop;
    MPI_Status status;

    if (rank == SCREEN_PROCESS) {
        DEBUG("[0] This is the screen process");
        Graphisme::Screen myScreen({ resx, resy },
                                   { grid.getLeftBottomVertex(), grid.getRightTopVertex() });

        while (myScreen.isOpen()) {
            auto start = std::chrono::system_clock::now();
            advance = false;
            // on inspecte tous les évènements de la fenêtre qui ont été émis depuis
            // la précédente itération
            sf::Event event;
            while (myScreen.pollEvent(event)) {
                // évènement "fermeture demandée" : on ferme la fenêtre
                if (event.type == sf::Event::Closed) {
                    DEBUG("[0] sending CLOSE");
                    ui_event = UiEvent::CloseWindow;
                    ui_event.send(SIM_PROCESS, comm);
                    DEBUG("[0] sent!");

                    myScreen.close();
                } else if (event.type == sf::Event::Resized) {
                    // on met à jour la vue, avec la nouvelle taille de la fenêtre
                    myScreen.resize(event);
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                    DEBUG(animate, "[0] sending START");
                    ui_event = UiEvent::AnimationStart;
                    ui_event.send(SIM_PROCESS, comm);
                    animate = true;
                    DEBUG(animate, "[0] sent!");
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                    DEBUG(animate, "[0] sending STOP");
                    ui_event = UiEvent::AnimationStop;
                    ui_event.send(SIM_PROCESS, comm);
                    animate = false;
                    DEBUG(animate, "[0] sent!");
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                    DEBUG(dt, "[0] sending TIMESTEP_INCREMENT");
                    ui_event = UiEvent::TimestepIncrement;
                    ui_event.send(SIM_PROCESS, comm);
                    dt *= 2;
                    DEBUG(dt, "[0] sent!");
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                    DEBUG(dt, "[0] sending TIMESTEP_DECREMENT");
                    ui_event = UiEvent::TimestepDecrement;
                    ui_event.send(SIM_PROCESS, comm);
                    dt /= 2;
                    DEBUG(dt, "[0] sent!");
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    DEBUG(advance, "[0] sending ADVANCE");
                    ui_event = UiEvent::Advance;
                    ui_event.send(SIM_PROCESS, comm);
                    advance = true;
                    DEBUG(advance, "[0] sent!");
                }
            }

            // we don't have to receive every time
            if (animate || advance) {
                DEBUG(animate || advance, "[0] advancing: waiting on recv()");
                if (isMobile) {
                    DEBUG(isMobile, "[0] waiting for vortices");
                    vortices.recv(SIM_PROCESS, comm, &status);
                }
                cloud.recv(SIM_PROCESS, comm, &status);
                grid.recv(SIM_PROCESS, comm, &status);
            }

            myScreen.clear(sf::Color::Black);
            std::string strDt = std::string("Time step : ") + std::to_string(dt);
            myScreen.drawText(
                strDt, Geometry::Point<double> { 50, double(myScreen.getGeometry().second - 96) });

            myScreen.displayVelocityField(grid, vortices);
            myScreen.displayParticles(grid, vortices, cloud);

            auto end = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::string str_fps = std::string("FPS : ") + std::to_string(1. / diff.count());
            myScreen.drawText(str_fps, Geometry::Point<double> {
                                           300, double(myScreen.getGeometry().second - 96) });
            myScreen.display();
        }
    }

    if (rank == SIM_PROCESS) {
        DEBUG("[1] This is the sim process");
        int flag;
        while (ui_event != UiEvent::CloseWindow) {
            advance = false;

            MPI_Iprobe(SCREEN_PROCESS, UiEvent::TAG, comm, &flag, &status);
            if (flag) {
                ui_event.recv(SCREEN_PROCESS, comm, &status);
                DEBUG(ui_event, "[1] Received ui event");
                if (ui_event == UiEvent::Advance) {
                    advance = true;
                } else if (ui_event == UiEvent::AnimationStart) {
                    animate = true;
                } else if (ui_event == UiEvent::AnimationStop) {
                    animate = false;
                } else if (ui_event == UiEvent::TimestepIncrement) {
                    dt *= 2;
                } else if (ui_event == UiEvent::TimestepDecrement) {
                    dt /= 2;
                } else if (ui_event == UiEvent::CloseWindow) {
                    DEBUG(rank, "[1] breaking");
                    break;
                }
            }

            if (animate || advance) {
                if (isMobile) {
                    cloud = Numeric::solve_RK4_movable_vortices(dt, grid, vortices, cloud);
                    vortices.send(SCREEN_PROCESS, comm);
                } else {
                    cloud = Numeric::solve_RK4_fixed_vortices(dt, grid, cloud);
                }

                DEBUG(errstr, "[1] sending cloud");
                int errcode = cloud.send(SCREEN_PROCESS, comm);

                if (errcode != MPI_SUCCESS)
                    MPI_Error_string(errcode, errstr, &errlen);
                DEBUG(errstr, "[1] sending grid");
                errcode = grid.send(SCREEN_PROCESS, comm);

                if (errcode != MPI_SUCCESS)
                    MPI_Error_string(errcode, errstr, &errlen);
                DEBUG(errstr, "[1] sent");
            }
        }
    }

    MPI_Barrier(comm);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
