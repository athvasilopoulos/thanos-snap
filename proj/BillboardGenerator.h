#ifndef BBOARD_GEN_H
#define BBOARD_GEN_H
class Drawable;
#include <glm/glm.hpp>
#include "Billboard.h"
#include "BoundingBox.h"
#include <vector>

class BillboardGenerator {
public:
    std::vector<std::vector<Billboard*>> billboards;
    int rows;
    float bboard_size;

    BillboardGenerator(std::vector<BoundingBox*> bbox, std::vector<std::vector<bool>> billboardMap, float size);

    void updateBillboards(glm::vec3 model_pos, glm::vec3 camera_pos);
    void newRow(int row);
    glm::vec4 calculateBillboardRotationMatrix(glm::vec3 particle_pos, glm::vec3 camera_pos);
    void removeBillboards();

};

#endif
