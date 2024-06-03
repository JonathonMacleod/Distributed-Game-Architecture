#version 430

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTextureCoords;

out vec2 v2fTextureCoord;

void main() {
	v2fTextureCoord = iTextureCoords;
	gl_Position = vec4(iPosition, 1);
}