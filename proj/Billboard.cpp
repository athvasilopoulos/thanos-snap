#include "Billboard.h"
#include <GL/glew.h>
#include <common/model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <omp.h>

using namespace glm;

Billboard::Billboard(vec3 starting_position, vec3 starting_speed, float side) {
	quad = new Drawable("models/quad.obj");
    pos = starting_position;
    speed = starting_speed;
    size = side;
    lives = 700;
}

// Draw the billboard
void Billboard::draw(unsigned int drawable) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    quad->bind();
    quad->draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
}
