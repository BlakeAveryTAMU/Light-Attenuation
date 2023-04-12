#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;
uniform float t;
attribute vec4 aPos; // In object space
attribute vec3 aNor; // In object space
attribute vec2 aTex;

varying vec3 vNor; // In camera space
varying vec2 vTex;
varying vec3 vPos;

void main()
{

	float x = aPos.x;
	float theta = aPos.y;

	vec3 position = vec3(x, (cos(x + t) + 2) * cos(theta), (cos(x + t) + 2) * sin(theta));

	vec3 dpdx = vec3(1, -1.0 * sin(x + t) * cos(theta), -1.0 * sin(x + t) * sin(theta));
	vec3 dpdtheta = vec3(0, -1.0 * (cos(x + t) + 2) * sin(theta), (cos(x + t) + 2) * cos(theta));
	vec3 normal = normalize(cross(dpdx, dpdtheta));

	gl_Position = P * (MV * vec4(position, 1));
	
	vPos = position;
	vNor = vec3(MVit * vec4(normal, 0.0)); // Assuming MV contains only translations and rotations
	vTex = aTex;


}