#version 430

in vec3 v2fColour;
in vec4 v2fClipSpacePosition;
in flat int v2fInstanceId;

layout (location = 600) uniform vec4 uDrawingBoundaries[100];

layout(location = 0) out vec3 oColour;

void main() {
	// Determine whether the current pixel is within the current drawing boundaries
	const vec4 drawingBoundary = uDrawingBoundaries[v2fInstanceId];

	const float drawingBoundaryX = drawingBoundary.x;
	const float drawingBoundaryY = drawingBoundary.y;
	const float drawingBoundaryWidth = drawingBoundary.z;
	const float drawingBoundaryHeight = drawingBoundary.w;
	
	const vec2 expectedScreenPosition = vec2(v2fClipSpacePosition.x / v2fClipSpacePosition.w, v2fClipSpacePosition.y / v2fClipSpacePosition.w);

	if((expectedScreenPosition.x < drawingBoundaryX) || (expectedScreenPosition.x > drawingBoundaryX + drawingBoundaryWidth)) discard;
	if((expectedScreenPosition.y < drawingBoundaryY) || (expectedScreenPosition.y > drawingBoundaryY + drawingBoundaryHeight)) discard;

	oColour = v2fColour;
}