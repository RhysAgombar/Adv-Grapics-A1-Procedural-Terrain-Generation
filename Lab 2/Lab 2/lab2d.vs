/*
 *  Simple vertex shader for lab 4
 */

#version 330 core

uniform mat4 modelView;
uniform mat4 projection;
in vec4 vPosition;
in vec3 vNormal;

uniform vec4 colour;
uniform vec3 Eye;
uniform vec3 light;
uniform vec4 material;

out vec4 colourO;

void main() {
	vec3 normal;
	vec3 position;

	gl_Position = projection * modelView * vPosition;
	position = vPosition.xyz;
	normal = vNormal;

	vec4 white = vec4(1.0,1.0,1.0,1.0);
	float diffuse;
	vec3 L = normalize(light);
	vec3 R = normalize(reflect(-L,normal));
	vec3 N;
	vec3 H = normalize(L + (Eye - position));
	float specular;

	N = normalize(normal);
	diffuse = dot(N,L);
	if (diffuse < 0.0){
		diffuse = 0.0;
		specular = 0.0;
	} else {
		specular =  pow(max(0.0, dot(N,R)),material.w);
	}

	colourO = min(material.x * colour + material.y * diffuse * colour + material.z * white * specular, vec4(1.0));

}