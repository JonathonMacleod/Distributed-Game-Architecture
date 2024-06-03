#version 430

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iColour;

layout (location = 0) uniform mat4 uViewMatrix[100];
layout (location = 200) uniform mat4 uModelMatrix;
layout (location = 400) uniform mat4 uProjectionMatrix[100];
layout (location = 600) uniform vec4 uDrawingBoundaries[100];

out vec3 v2fColour;
out vec4 v2fClipSpacePosition;
out flat int v2fInstanceId;

void main() {
	v2fInstanceId = gl_InstanceID;
	v2fColour = iColour;

	// Retrieve the area we want to draw this model to
	const vec4 drawingBoundary = uDrawingBoundaries[gl_InstanceID];

	// Work out some offsets and scaling multipliers to translate from a full clip space (-1,-1 to 1,1) to the drawing boundary (X,Y to X+W,Y+H)
	const float screenSpaceOffsetX = drawingBoundary.x;
	const float screenSpaceOffsetY = drawingBoundary.y;
	const float screenSpaceHorizontalScale = (drawingBoundary.z);
	const float screenSpaceVerticalScale = (drawingBoundary.w);

	// Work out where OpenGL would like to render this model (if only one perspective was being drawn across the entire screen)
	const vec4 suggestedClipSpacePosition = uProjectionMatrix[gl_InstanceID] * uViewMatrix[gl_InstanceID] * uModelMatrix * vec4(iPosition, 1);
	const vec2 expectedScreenPosition = vec2(suggestedClipSpacePosition.x / suggestedClipSpacePosition.w, suggestedClipSpacePosition.y / suggestedClipSpacePosition.w);

	// Work out where we now need that position to be instead
	const float newScreenSpacePositionX = screenSpaceOffsetX + (expectedScreenPosition.x * screenSpaceHorizontalScale);
	const float newScreenSpacePositionY = screenSpaceOffsetY + (expectedScreenPosition.y * screenSpaceVerticalScale);

	// Recalculate the clip-space position (adjusted to result in the correct position within the screen-space)
	const float newClipSpacePositionX = newScreenSpacePositionX * suggestedClipSpacePosition.w;
	const float newClipSpacePositionY = newScreenSpacePositionY * suggestedClipSpacePosition.w;

	// Set the position of this vertex to be at the clip-space (adjusted for the current drawing boundary)
	v2fClipSpacePosition = vec4(newClipSpacePositionX, newClipSpacePositionY, suggestedClipSpacePosition.zw);
	gl_Position = v2fClipSpacePosition;
}