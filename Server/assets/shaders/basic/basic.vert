#version 430

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iColour;
layout (location = 2) in vec2 iTextureCoords;

layout (location = 0) uniform mat4 uViewMatrix;
layout (location = 1) uniform mat4 uModelMatrix;
layout (location = 2) uniform mat4 uProjectionMatrix;

out vec3 v2fColour;

void main() {
	v2fColour = iColour;

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(iPosition, 1);
}