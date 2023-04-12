#ifndef OBJECT_H
#define OBJECT_H


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shape.h"
#include "MyShape.h"

#include <memory>
#include <iostream>
#include <string>
#include <random>

class Object {

private:

	std::string shape_type;
	std::shared_ptr<Shape> shape;
	std::shared_ptr<MyShape> mShape;
	glm::vec3 translation;
	glm::vec3 scale;
	glm::vec3 rotation;
	glm::vec3 diffuse;
	glm::vec3 emissive;

public:

	float random_scale_factor = (float)(rand()) / (float)(RAND_MAX) / 2.0 + 0.1f;
	float smaller_random_scale_factor = (float)(rand()) / (float)(RAND_MAX) / 2.0;

	Object() {
		shape_type = "";
		shape = nullptr;
		mShape = nullptr;
		translation = glm::vec3(0, 0, 0);
		scale = glm::vec3(random_scale_factor, random_scale_factor, random_scale_factor);
		rotation = glm::vec3(0, 0, 0);
		diffuse = glm::vec3((float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX));
		emissive = glm::vec3(0, 0, 0);
	}


	//OVerload the setShape function e.g void setShape(shared_ptr<MyShape> s)
	void setShape(std::shared_ptr<MyShape> m) { mShape = m; }
	std::shared_ptr<MyShape> getMyShape() { 
		GLSL::checkError(GET_FILE_LINE);
		return mShape; }

	void setShape(std::shared_ptr<Shape> s) {shape = s;}
	std::shared_ptr<Shape> getShape() { return shape; }

	void setTranslation(glm::vec3 v) { translation = v; }
	glm::vec3 getTranslation() { return translation; }

	void setScale(glm::vec3 s) { scale = s; }
	glm::vec3 getScale() { return scale; }

	void setRotation(glm::vec3 r) { rotation = r; }
	glm::vec3 getRotatoin() { return rotation; }

	glm::vec3 getDiffuse() { return diffuse; }
	
	glm::vec3 getEmissive() { return emissive; }

	std::string getShapeType() { return shape_type; }
	void setShapeType(std::string s) { shape_type = s; }


};

#endif