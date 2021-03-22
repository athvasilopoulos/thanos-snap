#ifndef BBOARD_H
#define BBOARD_H
class Drawable;
#include <glm/glm.hpp>
#include <vector>
class Billboard {
public:
    Drawable* quad;
    glm::vec3 pos;
    glm::vec3 speed;
    float size;
    int lives;
    glm::mat4 modelMatrix;

    Billboard(glm::vec3 pos, glm::vec3 speed, float size);
    //~Billboard();

    void draw(unsigned int drawable = 0);
};

#endif
