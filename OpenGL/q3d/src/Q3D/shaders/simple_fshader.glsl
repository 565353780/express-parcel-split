#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;
uniform mat4 model_matrix;

varying vec2 v_texcoord;
varying vec3 v_normal;
varying vec3 v_view_normal;
varying vec3 v_frag_pos;

//! [0]
void main()
{

    // Set fragment color from texture
//    gl_FragColor = vec4(vec3(1.0,0.8,0.2) *(dot(v_view_normal,normalize(v_frag_pos))),1.0);
//    gl_FragColor = vec4(vec3(v_view_normal + 1.0) * 0.5,1.0);

    gl_FragColor= texture2D(texture, v_texcoord);
}
//! [0]

