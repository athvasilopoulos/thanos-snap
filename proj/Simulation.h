#ifndef SIM_H
#define SIM_H

#include "Sphere.h"
#include "BoundingBox.h"
#include "GlobalVariables.h"
#include <vector>
#include <glm/glm.hpp>

// Global variables in main.cpp used in Simulation.cpp
extern std::vector<std::vector<BoundingBox*>> bbox;
extern std::vector<std::vector<Sphere*>> spheres;
extern bool sim[N];
extern bool dispersion[N];

// Function Prototypes
void checkSim(glm::vec3 position, float h_angle, float v_angle);
void removeSpheres();

#endif
