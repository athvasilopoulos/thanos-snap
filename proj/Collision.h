#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>

class Box;
class Sphere;
void handleFloorSphereCollision(Sphere& sphere);
void handleSpheresCollision(Sphere& sphere1, Sphere& sphere2);
#endif
