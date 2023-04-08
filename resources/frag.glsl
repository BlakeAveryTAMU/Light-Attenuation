#version 120

uniform vec3 lightDiffuseColors[10];
uniform vec3 lightPositions[10];
uniform vec3 ka;
uniform vec3 ke;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 vPos; // camera space position
varying vec3 vNor; // camera space normal



void main()
{

	vec3 fragColor = ke;

	for (int i = 0; i < 10; i++){
	
		vec3 lightDir = lightPositions[i] - vPos;
		lightDir = normalize(lightDir);
		float lambertian = max(0.0, dot(lightDir, normalize(vNor)));
		vec3 diffuse_color = kd * lambertian;

		vec3 eyeVector = normalize(-1 * vPos);
		vec3 halfDir = normalize(lightDir + eyeVector);
		float specular = pow(max(0.0, dot(halfDir, normalize(vNor))), s);
		vec3 specular_color = ks * specular;

		vec3 color = lightDiffuseColors[i] * (diffuse_color + specular_color);

		float r = distance(lightPositions[i], vPos);

		float attenuation = 1.0 / (1.0 + 0.0429 * r + 0.9857 * pow(r, 2));

		fragColor += color * attenuation;
	}

	gl_FragColor = vec4(fragColor, 1.0);
	
}
