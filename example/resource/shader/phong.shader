//-> VERTEX_SHADER {
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

out vec3 v_normal;
out vec3 v_frag_position; // in world space

void main()
{
    gl_Position = vec4(position, 1.0f) * ModelMatrix * ViewMatrix * ProjectionMatrix;
    v_normal = (vec4(normal, 1.0f) * ModelMatrix).xyz;
    v_frag_position = (vec4(position, 1.0f) * ModelMatrix).xyz;
}
//-> }

//-> FRAGMENT_SHADER {
#version 330 core

in vec3 v_frag_position;
in vec3 v_normal;
out vec4 color;

uniform vec3 LightColor;
uniform vec3 LightPos;
uniform vec3 ViewPos;
uniform vec3 ObjectColor;

void main()
{
    //
    vec3 ambient = ObjectColor * LightColor;

    //
    vec3 light_dir = normalize(LightPos - v_frag_position);
    vec3 norm = normalize(v_normal);
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * LightColor;

    //
    vec3 reflect_dir = reflect(-light_dir, norm);
    vec3 view_dir = normalize(ViewPos - v_frag_position);
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), 32) * LightColor;

    //
    color = vec4(specular*0.5f + diffuse*0.8f + ambient*0.3f, 1.0f);
}
//-> }