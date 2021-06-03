#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_color;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;
varying vec3 v_normal;
varying vec3 v_view_normal;
varying vec3 v_frag_pos;
//! [0]
void main()
{
    // Calculate vertex position in screen space
    gl_Position = mvp_matrix * vec4(a_position,1);

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
    v_normal = a_normal;
    vec4 normal = (model_matrix * vec4(a_normal,0.0));
    v_view_normal = normal.xyz;
    v_frag_pos = gl_Position.xyz;
}
//! [0]
