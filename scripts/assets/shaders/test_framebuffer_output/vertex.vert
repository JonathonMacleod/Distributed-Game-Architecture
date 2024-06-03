#version 430

layout (location = 0) in vec2 iPosition;
layout (location = 1) in vec3 iColour;
layout (location = 2) in vec2 iTextureCoords;

out vec2 v2fTextureCoords;

void main() {
	v2fTextureCoords = iTextureCoords;
	gl_Position = vec4(iPosition, 0, 1);
}