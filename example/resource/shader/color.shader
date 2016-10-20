//-> VERTEX_SHADER {
#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 lm_ProjectionMatrix;
uniform mat4 lm_ViewMatrix;
uniform mat4 lm_ModelMatrix;

void main()
{
    gl_Position = vec4(position, 1.0f) * lm_ModelMatrix * lm_ViewMatrix * lm_ProjectionMatrix;
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