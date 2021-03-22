#include "Billboard.h"
#include "BillboardGenerator.h"
#include "GlobalVariables.h"
#include <GL/glew.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace glm;

// Initialize the billboards for a model, using the billboard map
BillboardGenerator::BillboardGenerator(std::vector<BoundingBox*> bbox, std::vector<std::vector<bool>> billboardMap, float size) {
	bboard_size = size;
	rows = 0;
	int step = bboard_size * 100;
	float halfstep = 0.01f * step * 0.5f;
	for (int i = 0; i < 5; i++) {
		int cntr = 0;
		float z = (bbox[i]->limits[5] + bbox[i]->limits[4]) / 2;
		float rangex = bbox[i]->limits[1] - bbox[i]->limits[0];
		float rangey = bbox[i]->limits[3] - bbox[i]->limits[2];
		for (int j = 0; j < (int)(rangey * 100.0f); j += step) {
			std::vector<Billboard*> bboards;
			for (int k = 0; k < (int)(rangex * 100.0f); k += step) {
				if (billboardMap[i][cntr]) {
					vec3 center = vec3(bbox[i]->limits[0] + 0.01f * k, bbox[i]->limits[3] - 0.01f * j, z) + vec3(halfstep, -halfstep, 0);
					Billboard* temp = new Billboard(center, vec3(0.0f, 0.0f, 0.0f), bboard_size);
					bboards.push_back(temp);
				}
				cntr++;
			}
			billboards.push_back(bboards);
		}
	}
}

// Update the positions of the billboards
void BillboardGenerator::updateBillboards(vec3 model_pos, vec3 camera_pos) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < billboards[i].size(); j++) {
			billboards[i][j]->pos += billboards[i][j]->speed;
			billboards[i][j]->lives--;
			vec4 billboard_rot = calculateBillboardRotationMatrix(billboards[i][j]->pos + model_pos, camera_pos);
			billboards[i][j]->modelMatrix = translate(mat4(), billboards[i][j]->pos) * rotate(mat4(), billboard_rot.w + 3.14f,vec3(billboard_rot.x, billboard_rot.y, billboard_rot.z)) * scale(mat4(), vec3(bboard_size, bboard_size, bboard_size));
		}
	}
}

// Activate a new billboard row and give every billboard 
// a random speed
void BillboardGenerator::newRow(int row) {
	rows++;
	if (rows == billboards.size()) rows--;
	for (int i = 0; i < billboards[row].size(); i++) {
		float speed_x = (rand() / ((float)RAND_MAX)) - 0.5f;
		float speed_z = (rand() / ((float)RAND_MAX)) - 0.5f;
		if (speed_x > 0.0f) speed_x + 0.5f;
		else speed_x - 0.5f;
		if (speed_z > 0.0f) speed_z + 0.5f;
		else speed_z - 0.5f;
		billboards[row][i]->speed = 0.003f * vec3(speed_x, 1.0f, speed_z);
	}
}

// Remove the billboards with 0 lives left
void BillboardGenerator::removeBillboards() {
	for (int i = 0; i < rows; i++) {
		auto j = billboards[i].begin();
		while (j != billboards[i].end()) {
			if ((*j)->lives < 0) {
				j = billboards[i].erase(j);
			}
			else {
				++j;
			}
		}
	}
}

// Rotate the billboards according to the camera position
vec4 BillboardGenerator::calculateBillboardRotationMatrix(vec3 particle_pos, vec3 camera_pos){
	vec3 dir = camera_pos - particle_pos;
	dir.y = 0;
	dir = glm::normalize(dir);

	vec3 rot_axis = cross(glm::vec3(0, 0, 1), dir);
	float rot_angle = glm::acos(dot(vec3(0, 0, 1), dir));

	return vec4(rot_axis, rot_angle);
}