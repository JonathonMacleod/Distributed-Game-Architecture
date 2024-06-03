#version 430

in vec2 v2fTextureCoords;

layout(binding = 0) uniform sampler2D uTexture0;

layout(location = 0) out vec4 oColour;

void main() {
	oColour = texture(uTexture0, v2fTextureCoords);
}