#version 430 core

out vec4 FragColor;

in vec2 texCoord;
in vec3 Normal;
in vec3 crntPos;

uniform sampler2D baseTexture;
uniform vec3 color;
uniform vec3 camPos;

struct Light {
	vec3 loc;
	float strength;
	vec3 dir;
	uint lightType;
	vec3 color;		float pad1;
};

layout(std430, binding = 5) buffer Lights { Light lights[]; };

vec4 directLight() {
	return vec4(vec3(dot(-lights[0].dir, Normal)), 1.f);
}

vec4 pointLight()
{	
	float strength = 1.f;
	vec3 lightVec = lights[0].loc - crntPos;
	float dist = length(lightVec);
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	return vec4(vec3(diffuse * strength / dist), 1.f);
}

void main() {
	vec4 light;
	if (lights[0].lightType == 1)
		light = directLight();
	else
		light = pointLight();


	FragColor = light * texture(baseTexture, texCoord);
}