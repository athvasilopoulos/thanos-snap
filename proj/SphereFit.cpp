#include "Collision.h"
#include "Box.h"
#include "Sphere.h"
#include "SphereFit.h"
#include "common/model.h"
#include <omp.h>
#include <iostream>

using namespace glm;

/**
 * Create the 6 limits for each bounding box
 * The y-levels used were manually selected via 
 * trial and error and are specific for this model.
 */
void createLimitsArray() {
    // initialize limits with big numbers
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            if(j % 2 == 0)
                limits[i][j] = 1000;
            else
                limits[i][j] = -1000;
        }
    }

    for (int i = 0; i < models[0]->vertices.size(); i++) {
        if (models[0]->vertices[i].y > 1.0f) {
            if (models[0]->vertices[i].x < limits[0][0]) limits[0][0] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].x > limits[0][1]) limits[0][1] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].y < limits[0][2]) limits[0][2] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].y > limits[0][3]) limits[0][3] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].z < limits[0][4]) limits[0][4] = models[0]->vertices[i].z;
            if (models[0]->vertices[i].z > limits[0][5]) limits[0][5] = models[0]->vertices[i].z;
        }
        else if (models[0]->vertices[i].y > 0.5f) {
            if (models[0]->vertices[i].x < limits[1][0]) limits[1][0] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].x > limits[1][1]) limits[1][1] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].y < limits[1][2]) limits[1][2] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].y > limits[1][3]) limits[1][3] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].z < limits[1][4]) limits[1][4] = models[0]->vertices[i].z;
            if (models[0]->vertices[i].z > limits[1][5]) limits[1][5] = models[0]->vertices[i].z;
        }
        else if (models[0]->vertices[i].y > -0.2f) {
            if (models[0]->vertices[i].x < limits[2][0]) limits[2][0] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].x > limits[2][1]) limits[2][1] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].y < limits[2][2]) limits[2][2] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].y > limits[2][3]) limits[2][3] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].z < limits[2][4]) limits[2][4] = models[0]->vertices[i].z;
            if (models[0]->vertices[i].z > limits[2][5]) limits[2][5] = models[0]->vertices[i].z;
        }
        else if (models[0]->vertices[i].y > -1.4f) {
            if (models[0]->vertices[i].x < limits[3][0]) limits[3][0] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].x > limits[3][1]) limits[3][1] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].y < limits[3][2]) limits[3][2] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].y > limits[3][3]) limits[3][3] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].z < limits[3][4]) limits[3][4] = models[0]->vertices[i].z;
            if (models[0]->vertices[i].z > limits[3][5]) limits[3][5] = models[0]->vertices[i].z;
        }
        else {
            if (models[0]->vertices[i].x < limits[4][0]) limits[4][0] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].x > limits[4][1]) limits[4][1] = models[0]->vertices[i].x;
            if (models[0]->vertices[i].y < limits[4][2]) limits[4][2] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].y > limits[4][3]) limits[4][3] = models[0]->vertices[i].y;
            if (models[0]->vertices[i].z < limits[4][4]) limits[4][4] = models[0]->vertices[i].z;
            if (models[0]->vertices[i].z > limits[4][5]) limits[4][5] = models[0]->vertices[i].z;
        }
    }
}

/**
 * A Moller–Trumbore ray-triangle intersection algorithm implementation
 * taken by http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
 */
bool MollerTrumbore(vec3 point, vec3 v0, vec3 v1, vec3 v2) {
    const float epsilon = 0.0000001;
    vec3 edge1, edge2, h, s, q;
    float a, f, u, v;
    vec3 rayDir = vec3(0.0f, 0.0f, 1.0f);

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

/**
 * Decide if a point is inside the model using ray - tringle intersection
 * with every triangle in the same bounding box. The algorithm used for the 
 * ray - triangle intersection is the Moller–Trumbore algorithm.
 */
bool point_inside(vec3 point, int bboxID) {
    int intersect = 0;
    for (int i = 0; i < bbox[0][bboxID]->vertices.size(); i += 3) {
        if (MollerTrumbore(point, bbox[0][bboxID]->vertices[i], bbox[0][bboxID]->vertices[i + 1], bbox[0][bboxID]->vertices[i + 2]))
            intersect++;
    }
    if (intersect % 2 == 0) return false;
    else return true;
}

/**
 * Create six boundary points of a sphere and check if each one of them
 * is inside the model. This function uses OpenMP to accelerate the process
 */
bool sphere_inside(vec3 center, float rad,int bboxID) {
    vec3 up = center + vec3(0.0f, rad, 0.0f);
    vec3 down = center + vec3(0.0f, -rad, 0.0f);
    vec3 front = center + vec3(0.0f, 0.0f, rad);
    vec3 back = center + vec3(0.0f, 0.0f, -rad);
    vec3 right = center + vec3(rad, 0.0f, 0.0f);
    vec3 left = center + vec3(-rad, 0.0f, 0.0f);
    vec3 points[6] = { up, down, front, back, right, left };
    
    bool inside = true;
    #pragma omp parallel for reduction(&&:inside)
    for (int i = 0; i < 6; i++) {
        bool temp = point_inside(points[i], bboxID);
        inside = inside && temp;
    }
    return inside;
}

// Initialize the speed of each sphere depending on its position
vec3 appendStartingSpeed(vec3 center) {
    static float x_med = (limits[0][0] + limits[0][1]) / 2.0f;
    static float z_med = (limits[0][4] + limits[0][5]) / 2.0f;
    vec3 speed;
    if (center.x > x_med)
        speed.x = 0.5f;
    else
        speed.x = -0.5f;

    if (center.z > z_med)
        speed.z = 0.5f;
    else
        speed.z = -0.5f;

    speed.y = 1.0f;
    return speed;
}

/**
 * Traverse the bounding boxes in cubes of size equal to step/100 and 
 * check if the sphere of the radious given and with the same center as the cube
 * is inside the model. If so, create the sphere and push it in the spheres vector.
 */
void createSpheres(int step, float *rad, float mass) {
    float halfstep = 0.01f * step * 0.5f;
    for (int i = 0; i < 5; i++) {
        float rangex = bbox[0][i]->limits[1] - bbox[0][i]->limits[0];
        float rangey = bbox[0][i]->limits[3] - bbox[0][i]->limits[2];
        float rangez = bbox[0][i]->limits[5] - bbox[0][i]->limits[4];
        for (int j = 0; j < (int)(rangex * 100.0f) - step; j += step) {
            for (int k = 0; k < (int)(rangey * 100.0f) - step; k += step) {
                for (int l = 0; l < (int)(rangez * 100.0f) - step; l += step) {
                    vec3 bot_left_back = vec3(bbox[0][i]->limits[0] + 0.01f * j, bbox[0][i]->limits[2] + 0.01f * k, bbox[0][i]->limits[4] + 0.01f * l);
                    vec3 center = bot_left_back + vec3(halfstep, halfstep, halfstep);
                    if (sphere_inside(center, rad[0], i)) {
                        for (int s = 0; s < N; s++) {
                            spheres[s].push_back(new Sphere(center, appendStartingSpeed(center), rad[0], mass));
                            spheresStartingHeight[s].push_back(center.y);
                        }
                    }
                    else if (sphere_inside(center, rad[1], i)) {
                        for (int s = 0; s < N; s++) {
                            spheres[s].push_back(new Sphere(center, appendStartingSpeed(center), rad[1], mass));
                            spheresStartingHeight[s].push_back(center.y);
                        }
                    }
                    else if (sphere_inside(center, rad[2], i)) {
                        for (int s = 0; s < N; s++) {
                            spheres[s].push_back(new Sphere(center, appendStartingSpeed(center), rad[2], mass));
                            spheresStartingHeight[s].push_back(center.y);
                        }
                    }
                }
            }
        }
    }
}

// Create a billboard map to use for the billboard generator
void createBillboardMap(float bboard_size) {
    int step = bboard_size * 100;
    float halfstep = 0.01f * step * 0.5f;
    for (int i = 0; i < 5; i++) {
        // Cut the model in half
        float z = (bbox[0][i]->limits[5] + bbox[0][i]->limits[4]) / 2;
        float rangex = bbox[0][i]->limits[1] - bbox[0][i]->limits[0];
        float rangey = bbox[0][i]->limits[3] - bbox[0][i]->limits[2];
        for (int j = 0; j < (int)(rangey * 100.0f); j += step) {
            b_levels.push_back(bbox[0][i]->limits[3] - 0.01f * j);
            for (int k = 0; k < (int)(rangex * 100.0f); k += step) {
                vec3 center = vec3(bbox[0][i]->limits[0] + 0.01f * k, bbox[0][i]->limits[3] - 0.01f * j, z) + vec3(halfstep, -halfstep, 0);
                if (point_inside(center, i))
                    billboardMap[i].push_back(true);
                else
                    billboardMap[i].push_back(false);
            }
        }
    }
}
