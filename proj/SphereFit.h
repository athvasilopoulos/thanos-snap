#ifndef SPHERE_FIT_H
#define SPHERE_FIT_H

#include "Sphere.h"
#include "BoundingBox.h"
#include "GlobalVariables.h"
#include <vector>
#include <glm/glm.hpp>
// Global variables in main.cpp used in SphereFit.cpp
extern std::vector<std::vector<BoundingBox*>> bbox;
extern float limits[5][6];
extern std::vector<Drawable*> models;
extern std::vector<std::vector<Sphere*>> spheres;
extern std::vector<std::vector<float>> spheresStartingHeight;
extern std::vector<std::vector<bool>> billboardMap;
extern std::vector<float> b_levels;

// Function Prototypes
void createLimitsArray();
void createBillboardMap(float bboard_size);
void createSpheres(int step, float *rad, float mass);

#endif
