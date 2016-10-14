/*
 *  Simple fragment sharder for Lab 2
 */

#version 330 core

in vec4 colourO;

void main() {

	gl_FragColor = colourO;
	gl_FragColor.a = colourO.a;
	
}