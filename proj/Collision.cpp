#include "Collision.h"
#include "Box.h"
#include "Sphere.h"

using namespace glm;

// Function declarations
void handleSpheresCollision(Sphere& sphere1, Sphere& sphere2);
bool checkForSpheresCollision(glm::vec3& pos1, const float& r1, glm::vec3& pos2, const float& r2, glm::vec3& n);
bool checkForFloorSphereCollision(vec3& pos, const float& r, vec3& n);

// Check and handle sphere to sphere collisions
void handleSpheresCollision(Sphere& sphere1, Sphere& sphere2) {
    vec3 n;
    if (checkForSpheresCollision(sphere1.x, sphere1.r, sphere2.x, sphere2.r, n)) {
        vec3 vel1x, vel1y, vel2x, vel2y;
        //sphere 1
        vel1x = n * glm::dot(n, sphere1.v);
        vel1y = sphere1.v - vel1x;
        //sphere 2
        n = -n;
        vel2x = n * glm::dot(n, sphere2.v);
        vel2y = sphere2.v - vel2x;

        float m1 = sphere1.m;
        float m2 = sphere2.m;
        sphere1.v = vel1x * (m1 - m2) / (m1 + m2) + vel2x * (2.f * m2) / (m1 + m2) + vel1y;
        sphere2.v = vel1x * (2.f * m1) / (m1 + m2) + vel2x * (m2 - m1) / (m1 + m2) + vel2y;
        sphere1.P = (m1 * sphere1.v) * 0.9f;
        sphere2.P = (m2 * sphere2.v) * 0.9f;
    }
}

// Check if two spheres collided
bool checkForSpheresCollision(vec3& pos1, const float& r1, vec3& pos2, const float& r2, vec3& n) {
    vec3 dir;
    dir.x = (pos1.x - pos2.x);
    dir.y = (pos1.y - pos2.y);
    dir.z = (pos1.z - pos2.z);
    float dist = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);

    if (dist <= r1 + r2) {
        pos1 = pos1 + normalize(dir) * (r1 + r2 - dist) / 2.0f;
        pos2 = pos2 - normalize(dir) * (r1 + r2 - dist) / 2.0f;
        n = normalize(dir);
        return true;
    }
    else
        return false;
}

// Check and handle sphere to floor collisions
void handleFloorSphereCollision(Sphere& sphere) {
    vec3 n;
    if (checkForFloorSphereCollision(sphere.x, sphere.r, n)) {
        sphere.v = sphere.v - n * glm::dot(sphere.v, n) * 2.0f;
        sphere.P = (sphere.m * sphere.v) * 0.9f;
    }
}

// Check for floor collision
bool checkForFloorSphereCollision(vec3& pos, const float& r, vec3& n) {
    if (pos.y - r <= 0) {
        //correction
        float dis = -(pos.y - r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, -1, 0);
        return true;
    }
    else
        return false;
}

