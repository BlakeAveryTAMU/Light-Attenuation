#include "MyShape.h"
#include "GLSL.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <iostream>




MyShape::MyShape() : indCount(0) {}

void MyShape::loadShape(std::string type) {


	float num_grid_points_x = 50;
	float num_grid_points_y = 50;

	float min_x = -0.5;
	float min_y = -0.5;

	float curr_x = min_x;
	float curr_y = min_y;

	float texture_x = 0.0;
	float texture_y = 0.0;

	float theta = 0.0;
	float phi = 0.0;

	float x = 0.0;

	if (type == "sphere") {

		for (int i = 0; i < num_grid_points_y; i++) { // rows

			texture_x = 0.0;
			theta = 0.0;

			for (int j = 0; j < num_grid_points_x; j++) { // columns


				posBuf.push_back(sin(theta) * sin(phi)); // Sphere
				posBuf.push_back(cos(theta));
				posBuf.push_back(sin(theta) * cos(phi));

				norBuf.push_back(sin(theta) * sin(phi)); // Sphere
				norBuf.push_back(cos(theta));
				norBuf.push_back(sin(theta) * cos(phi));

				texBuf.push_back(texture_x);
				texBuf.push_back(texture_y);

				texture_x = texture_x + 1 / (num_grid_points_x - 1);

				theta = theta + M_PI / (num_grid_points_x - 1);

			}
			phi = phi + 2 * M_PI / (num_grid_points_y - 1);
			texture_y = texture_y + 1 / (num_grid_points_y - 1);
		}
	}

	if (type == "vase") {

		for (int i = 0; i < num_grid_points_y; i++) { // rows

			texture_x = 0.0;
			x = 0.0;

			for (int j = 0; j < num_grid_points_x; j++) { // columns


				posBuf.push_back(x); // Sphere
				posBuf.push_back(theta);
				posBuf.push_back(0.0f);

				norBuf.push_back(0.0f); // Sphere
				norBuf.push_back(0.0f);
				norBuf.push_back(0.0f);

				texBuf.push_back(texture_x);
				texBuf.push_back(texture_y);

				texture_x = texture_x + 1.0 / (num_grid_points_x - 1);

				x = x + 10.0 / (num_grid_points_x - 1);

			}
			theta = theta + 2 * M_PI / (num_grid_points_x - 1);
			texture_y = texture_y + 1 / (num_grid_points_y - 1);
		}
	}

	

	int count = 0;
	for (int i = 0; i < num_grid_points_y - 1; i++) {

		for (int j = 0; j < num_grid_points_x - 1; j++) {

			indBuf.push_back(j + count);
			indBuf.push_back((j + 1) + count);
			indBuf.push_back((j + 1) + num_grid_points_x + count);

			indBuf.push_back(j + count);
			indBuf.push_back((j + 1) + num_grid_points_x + count);
			indBuf.push_back((j + num_grid_points_x) + count);
		}
		count += num_grid_points_x;
	}
}

void MyShape::init() {

	// Total number of indices
	indCount = (int)indBuf.size();

	// Generate buffer IDs and put them in the bufIDs map.
	GLuint tmp[4];
	glGenBuffers(4, tmp);
	bufIDs["bPos"] = tmp[0];
	bufIDs["bNor"] = tmp[1];
	bufIDs["bTex"] = tmp[2];
	bufIDs["bInd"] = tmp[3];
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size() * sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bTex"]);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIDs["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size() * sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	assert(norBuf.size() == posBuf.size());
	GLSL::checkError(GET_FILE_LINE);
}

void MyShape::draw(std::shared_ptr<Program> prog, std::string type, float t)  {

	if (type == "sphere") {

		glEnableVertexAttribArray(prog->getAttribute("aPos"));

		glEnableVertexAttribArray(prog->getAttribute("aNor"));

		//glEnableVertexAttribArray(prog->getAttribute("aTex"));

		glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);

		glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);

		glVertexAttribPointer(prog->getAttribute("aNor"), 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bTex"]);

		//glVertexAttribPointer(prog->getAttribute("aTex"), 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIDs["bInd"]);

		glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_INT, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//glDisableVertexAttribArray(prog->getAttribute("aTex"));

		glDisableVertexAttribArray(prog->getAttribute("aNor"));

		glDisableVertexAttribArray(prog->getAttribute("aPos"));
		GLSL::checkError(GET_FILE_LINE);
	}

	if (type == "vase") {
		//std::cout << "vase" << std::endl;

		glUniform1f(prog->getUniform("t"), t);
		
		glEnableVertexAttribArray(prog->getAttribute("aPos"));

		//glEnableVertexAttribArray(prog->getAttribute("aNor"));

		//glEnableVertexAttribArray(prog->getAttribute("aTex"));

		glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
		
		glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
		
		//glVertexAttribPointer(prog->getAttribute("aNor"), 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bTex"]);
		
		//glVertexAttribPointer(prog->getAttribute("aTex"), 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIDs["bInd"]);
		
		glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_INT, (void*)0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		//glDisableVertexAttribArray(prog->getAttribute("aTex"));

		//glDisableVertexAttribArray(prog->getAttribute("aNor"));

		glDisableVertexAttribArray(prog->getAttribute("aPos"));
		GLSL::checkError(GET_FILE_LINE);
	}
	
}

