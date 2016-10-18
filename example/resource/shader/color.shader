//-> VERTEX_SHADER {
#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main()
{
    gl_Position = vec4(position, 1.0f) * ModelMatrix * ViewMatrix * ProjectionMatrix;
}
//-> }

//-> FRAGMENT_SHADER {
#version 330 core

uniform vec3 ObjectColor;
out vec4 color;

void main()
{
    color = vec4(ObjectColor, 1.0f);
}
//-> }