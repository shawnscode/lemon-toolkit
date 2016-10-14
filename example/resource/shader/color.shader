//-> VERTEX_SHADER {
#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = vec4(position, 1.0f) * model * view * projection;
}
//-> }

//-> FRAGMENT_SHADER {
#version 330 core

uniform vec3 object_color;

out vec4 color;

void main()
{
    color = vec4(object_color, 1.0f);
}
//-> }