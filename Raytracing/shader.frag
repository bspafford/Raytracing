#version 430 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

uniform vec3 color;

void main() {
	vec3 lightDir = vec3(1.f, 0.75f, -0.5f);
	FragColor = vec4(vec3(dot(lightDir, Normal)), 1.f);
	FragColor = vec4(1, 1, 0, 1);
}