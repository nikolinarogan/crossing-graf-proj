#version 330 core

in vec2 chTex;
uniform sampler2D uTex;
uniform vec3 uColor;
out vec4 FragColor;

void main()
{
	vec4 sampled = texture(uTex, chTex);
	FragColor = vec4(uColor, 1.0) * sampled; //bojenje, toniranje
}
