#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light {

private:

	glm::vec3 position;
	glm::vec3 diffuse;

public:

	Light() {
		position = glm::vec3();
		diffuse = glm::vec3();
	}

	void setPosition(glm::vec3 p) {
		position = p;
	}
	void setDiffuse(glm::vec3 d) {
		diffuse = d;
	}

	glm::vec3 getPosition() { return position; }
	glm::vec3 getDiffuse() { return diffuse; }

	void translatePosition_X(float t) {
		position[0] += t;
	}

	void translatePosition_Y(float t) {
		position[1] += t;
	}

};


#endif