#include "Sphere.h"
#include "GlobalVariables.h"
#include "Simulation.h"
#include <vector>
#include <iostream>

using namespace glm;

// Remove spheres if the fall below an Energy threshold
void removeSpheres() {
    for (int i = 0; i < spheres.size(); i++) {
        auto j = spheres[i].begin();
        while (j != spheres[i].end()) {
            float energy = (*j)->x.y * g_earth * (*j)->m + (*j)->calcKinecticEnergy();
            if (energy < 0.2f) {
                j = spheres[i].erase(j);
            }
            else {
                ++j;
            }
        }
    }
}

/**
 * A Moller–Trumbore ray-triangle intersection algorithm implementation
 * taken by http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
 */
bool MollerTrumbore(vec3 point, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2) {
    const float epsilon = 0.0000001;
    vec3 edge1, edge2, h, s, q;
    float a, f, u, v;

    edge1 = v1 - v0;
    edge2 = v2 - v0;
    h = cross(rayDir, edge2);
    a = dot(edge1, h);
    if (a > -epsilon && a < epsilon)
        return false;

    f = 1.0f / a;
    s = point - v0;
    u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    q = cross(s, edge1);
    v = f * dot(rayDir, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * dot(edge2, q);
    if (t > epsilon)
        return true;
    else
        return false;
}

// Check if the user targeted a model and start the correct simulation
void checkSim(vec3 position, float h_angle, float v_angle) {
    vec3 direction(
        cos(v_angle) * sin(h_angle),
        sin(v_angle),
        cos(v_angle) * cos(h_angle)
    );
    float mindist = 1000;
    int id = N;
    for (int i = 0; i < bbox.size(); i++) {
        if (sim[i]) continue;
        bool hit = false;
        for (int j = 0; j < bbox[i].size(); j++) {
            for (int k = 0; k < bbox[i][j]->BoxVertices.size(); k += 3) {
                hit = MollerTrumbore(position, direction, bbox[i][j]->BoxVertices[k], bbox[i][j]->BoxVertices[k + 1], bbox[i][j]->BoxVertices[k + 2]);
                if (hit) {
                    float dist = distance(position, bbox[i][j]->BoxVertices[k]);
                    if (dist < mindist) {
                        id = i;
                        mindist = dist;
                    }
                    break;
                }
            }
            if (hit) break;
        }
    }
    if (id < N) {
        sim[id] = true;
        dispersion[id] = true;
    }
        
}
