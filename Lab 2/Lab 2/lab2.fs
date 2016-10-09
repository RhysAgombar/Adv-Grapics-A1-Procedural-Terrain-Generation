/*
 *  Simple fragment sharder for Lab 2
 */

#version 330 core

in vec3 normal;
in vec4 pos;


void main() {
	vec4 base = vec4(1.0, 0.0, 0.0, 1.0);
	vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
	vec3 lPos = vec3(0.5,1.0,6.1);
	float diffuse;
	float specular;
	vec3 N;

	vec3 L = lPos - pos.xyz;   //vec3(1.0, 0.0, 0.0);
	L = normalize(L);
	
	vec3 H = normalize(L + vec3(0.0, 0.0, -1.0));

	//L = normalize(L);
	
	N = normalize(normal);
	diffuse = dot(N,L);
	if(diffuse < 0.0) {
		diffuse = 0.0;
		specular = 0.0;
	} else {
		specular = pow(max(0.0, dot(N,H)),500.0);
	}

	//gl_FragColor = min(0.3*base + 0.7*diffuse*base + 0.7*white*specular, vec4(1.0));
	
	gl_FragColor = min(0.2*base + 0.85*diffuse*base*white + white*specular, vec4(1.0));

	gl_FragColor.a = base.a;

}