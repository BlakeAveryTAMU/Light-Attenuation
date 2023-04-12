#ifndef MY_SHAPE
#define MY_SHAPE

#define _USE_MATH_DEFINES
#include <cmath>

#include "Program.h"

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class MyShape {

private:

	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<unsigned int> indBuf;
	std::map<std::string, GLuint> bufIDs;
	int indCount;

public:

	MyShape();
	void loadShape(std::string type);
	void init();
	void draw( std::shared_ptr<Program> prog, std::string type, float t);

};

#endif