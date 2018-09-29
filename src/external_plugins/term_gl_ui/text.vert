#version 330 core
uniform sampler2D tex;
uniform vec3 pixel;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 vertex;
in vec4 color;
in vec2 tex_coord;
in float ashift;
in float agamma;

out vec4 vcolor;
out vec2 vtex_coord;
out float vshift;
out float vgamma;

void main()
{
    vshift = ashift;
    vgamma = agamma;
    vcolor = color;
    vtex_coord = tex_coord;
    gl_Position = projection*(view*(model*vec4(vertex,1.0)));
}
