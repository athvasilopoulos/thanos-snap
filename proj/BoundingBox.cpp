#include "BoundingBox.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/model.h>

using namespace glm;

BoundingBox::BoundingBox(float lim[]) {
    memcpy(limits, lim, 6 * sizeof(float));
    // Manual parallelogram creation using the Drawable class
    vec3 top_left_front = vec3(limits[0], limits[3], limits[5]);
    vec3 top_left_back = vec3(limits[0], limits[3], limits[4]);
    vec3 top_right_front = vec3(limits[1], limits[3], limits[5]);
    vec3 top_right_back = vec3(limits[1], limits[3], limits[4]);

    vec3 bot_left_front = vec3(limits[0], limits[2], limits[5]);
    vec3 bot_left_back = vec3(limits[0], limits[2], limits[4]);
    vec3 bot_right_front = vec3(limits[1], limits[2], limits[5]);
    vec3 bot_right_back = vec3(limits[1], limits[2], limits[4]);

    vec3 up_normal = vec3(0.0f, 1.0f, 0.0f);
    vec3 front_normal = vec3(0.0f, 0.0f, 1.0f);
    vec3 left_normal = vec3(-1.0f, 0.0f, 0.0f);
    vec3 back_normal = vec3(0.0f, 0.0f, -1.0f);
    vec3 right_normal = vec3(1.0f, 0.0f, 0.0f);
    vec3 down_normal = vec3(0.0f, -1.0f, 0.0f);

    const std::vector<vec3> bboxVert{
        top_left_front,     //panw
        top_left_back,
        top_right_back,

        top_left_front,
        top_right_front,
        top_right_back,

        top_left_front,     //mprosta
        top_right_front,
        bot_left_front,

        top_right_front,
        bot_left_front,
        bot_right_front,

        top_left_front,     //aristera
        top_left_back,
        bot_left_back,

        top_left_front,
        bot_left_back,
        bot_left_front,

        top_left_back,      //pisw
        top_right_back,
        bot_left_back,

        top_right_back,
        bot_left_back,
        bot_right_back,

        top_right_front,    //deksia
        top_right_back,
        bot_right_front,

        top_right_back,
        bot_right_front,
        bot_right_back,

        bot_left_front,     //katw
        bot_left_back,
        bot_right_back,

        bot_left_front,
        bot_right_front,
        bot_right_back,
    };
    const std::vector<vec2> bboxUVs;
    const std::vector<vec3> bboxNorm{
        up_normal,
        up_normal,
        up_normal,
        up_normal,
        up_normal,
        up_normal,

        front_normal,
        front_normal,
        front_normal,
        front_normal,
        front_normal,
        front_normal,

        left_normal,
        left_normal,
        left_normal,
        left_normal,
        left_normal,
        left_normal,

        back_normal,
        back_normal,
        back_normal,
        back_normal,
        back_normal,
        back_normal,

        right_normal,
        right_normal,
        right_normal,
        right_normal,
        right_normal,
        right_normal,

        down_normal,
        down_normal,
        down_normal,
        down_normal,
        down_normal,
        down_normal
    };
    
    parallelogram = new Drawable(bboxVert, bboxUVs, bboxNorm);

    BoxVertices = bboxVert;
    
}

BoundingBox::~BoundingBox() {
    delete parallelogram;
}

// Draw the bounding boxes
void BoundingBox::draw(unsigned int drawable) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    parallelogram->bind();
    parallelogram->draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
}

// Initialize the bounding box vertices array with the vertices of the model
// that are inside the box
void BoundingBox::fillVertices(std::vector<glm::vec3> modelVertices) {
    for (int i = 0; i < modelVertices.size(); i += 3) {
        bool add = false;
        if (modelVertices[i].y > limits[2] && modelVertices[i].y < limits[3]) add = add || true;
        if (modelVertices[i+1].y > limits[2] && modelVertices[i+1].y < limits[3]) add = add || true;
        if (modelVertices[i+2].y > limits[2] && modelVertices[i+2].y < limits[3]) add = add || true;
        if (add) {
            vertices.push_back(modelVertices[i]);
            vertices.push_back(modelVertices[i+1]);
            vertices.push_back(modelVertices[i+2]);
        }
    }
}
