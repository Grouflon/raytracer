#version 120

uniform vec2 uScreenSize;
uniform sampler2D uTexture;

varying vec2 vTexCoord;

void main()
{
	gl_FragColor = texture2D(uTexture, vTexCoord);
	//gl_FragColor = vec4(gl_FragCoord.xy / uScreenSize, 0, 1);
}