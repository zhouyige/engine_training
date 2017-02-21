#version 150

uniform mat4 u_projectionMatrix;

out vec2 texcoord;
in vec4 a_vertex;
in vec2 a_texcoord;;
void main(void)
{
	texcoord = a_texcoord;
	gl_Position = u_projectionMatrix * a_vertex;
}
