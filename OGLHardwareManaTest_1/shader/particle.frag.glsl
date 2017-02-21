#version 150

uniform sampler2D u_texture; 
in vec2 texcoord;
out vec4 fragColor;

void main(void)
{
	// Red fading out sprite
	fragColor = texture(u_texture, texcoord.xy);
}
