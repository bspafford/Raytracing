#version 430 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

uniform sampler2D tex0;
uniform vec3 lightDir;

void main() {
	float ambient = 0.2f;
	vec3 normal = normalize(Normal);
	float brightness = max(dot(normal, lightDir), 0.f) + ambient;
	vec4 color = texture(tex0, texCoord);
	FragColor = vec4(color.xyz * brightness, 1.f);
}