#version 120

attribute vec2 aPosition;
varying vec2 vTexCoord;

void main(void)
{
	gl_Position = vec4(aPosition, 0, 1.0);
	vTexCoord = vec2((aPosition.x + 1.0) / 2.0, (aPosition.y + 1.0) / 2.0);
}