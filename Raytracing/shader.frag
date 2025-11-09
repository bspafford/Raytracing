#version 430 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

uniform vec3 color;

void main() {
	FragColor = vec4(color, 1.f);
}