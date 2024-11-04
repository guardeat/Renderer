#version 330 core

void main() {
	gl_FragDepth = gl_FragColor.z;
}