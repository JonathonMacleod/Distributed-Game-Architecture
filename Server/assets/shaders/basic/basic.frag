#version 430

in vec3 v2fColour;

layout(location = 0) out vec3 oColour;

void main() {
	oColour = v2fColour;
}