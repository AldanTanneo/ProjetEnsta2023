# Configuration pour Linux
# Peut être modifié à votre convenance...
CXXFLAGS = -std=c++20
ifdef DEBUG
CXXFLAGS += -g -O0 -Wall -fbounds-check -pedantic -fsanitize=undefined,address -fopenmp
CXXFLAGS2 = CXXFLAGS
else
CXXFLAGS2 = ${CXXFLAGS} -O2 -march=native -Wall -fopenmp
CXXFLAGS += -O3 -march=native -Wall -fopenmp
endif
LIB=-lsfml-graphics -lsfml-window -lsfml-system
