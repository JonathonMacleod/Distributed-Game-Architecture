#version 430

in vec2 v2fTextureCoord;

uniform sampler2D texture1;

layout(location = 0) out vec3 oColour;

void main() {
	oColour = texture(texture1, v2fTextureCoord).xyz;
}