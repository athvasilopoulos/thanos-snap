#ifndef BBOX_H
#define BBOX_H

#include <glm/glm.hpp>
#include <vector>
class Drawable;

/**
 * Represents the bounding box
 */
class BoundingBox {
public:
    Drawable* parallelogram;
    float limits[6];
    glm::mat4 modelMatrix;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> BoxVertices;

    BoundingBox(float lim[]);
    ~BoundingBox();

    void draw(unsigned int drawable = 0);
    void BoundingBox::fillVertices(std::vector<glm::vec3> modelVertices);
};

#endif
