#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
	
out vec2 TexCoords;

uniform mat4 proj;
	
void main()
{
    TexCoords = aTexCoords;
    gl_Position = proj * vec4(aPos, 0.f, 1.f);
}