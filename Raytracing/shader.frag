#version 430 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

uniform sampler2D baseTexture;
uniform vec3 color;
uniform vec3 camPos;

vec3 lightDir = vec3(1.f, 0.75f, -0.5f);
vec3 lightPos = vec3(0, 3, 0);

vec4 directLight() {
	return vec4(vec3(dot(lightDir, Normal)), 1.f);
}

vec4 pointLight()
{	
	float strength = 10.f;
	vec3 lightVec = lightPos - crntPos;
	float dist = length(lightVec);
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	return vec4(diffuse * 1.f / dist * strength);
}

void main() {
	FragColor = pointLight() * texture(baseTexture, texCoord);
}