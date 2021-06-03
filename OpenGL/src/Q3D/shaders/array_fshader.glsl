#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif
uniform sampler2D back_texture;
uniform sampler2D texture;
uniform mat4 model_matrix;

varying vec2 v_texcoord;
varying vec3 v_normal;
varying vec3 v_view_normal;
varying vec3 v_frag_pos;

uniform vec2 u_texcoord;
uniform float u_width;

//! [0]
void main()
{
    if(length(v_texcoord - u_texcoord) < 1.0 / u_width)
    {
        gl_FragColor = texture2D(texture,v_texcoord);
    }
    else
    {
        gl_FragColor = texture2D(back_texture, v_texcoord);
    }
}
//! [0]

