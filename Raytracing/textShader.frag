#version 430 core
out vec4 FragColor;
	
in vec2 TexCoords;
	
uniform sampler2D tex;
	
void main()
{             
    float text = texture(tex, TexCoords).r;   
    FragColor = vec4(text.r);
}