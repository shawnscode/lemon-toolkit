//-> VERTEX_SHADER {
#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

uniform mat4 lm_ProjectionMatrix;
uniform mat4 lm_ViewMatrix;
uniform mat4 lm_ModelMatrix;
uniform mat3 lm_NormalMatrix;

out vec3 v_normal;
out vec3 v_position; // in world space

void main()
{
    gl_Position = vec4(Position, 1.0f) * lm_ModelMatrix * lm_ViewMatrix * lm_ProjectionMatrix;
    v_normal = Normal * lm_NormalMatrix;
    v_position = (vec4(Position, 1.0f) * lm_ModelMatrix).xyz;
}
//-> }

//-> FRAGMENT_SHADER {
#version 330 core

in vec3 v_position;
in vec3 v_normal;
out vec4 color;

uniform vec3 lm_ViewPos;
uniform vec3 LightColor;
uniform vec3 LightPos;
uniform vec3 ObjectColor;

void main()
{
    //
    vec3 ambient = ObjectColor * LightColor;

    //
    vec3 light_dir = normalize(LightPos - v_position);
    vec3 norm = normalize(v_normal);
    vec3 diffuse = max(dot(norm, light_dir), 0.0) * LightColor;

    //
    vec3 reflect_dir = reflect(-light_dir, norm);
    vec3 view_dir = normalize(lm_ViewPos - v_position);
    vec3 specular = pow(max(dot(view_dir, reflect_dir), 0.0), 32) * LightColor;

    //
    color = vec4(specular*0.5f + diffuse*0.8f + ambient*0.3f, 1.0f);
}
//-> }