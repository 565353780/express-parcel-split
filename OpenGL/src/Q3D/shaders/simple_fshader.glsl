#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif
uniform int u_use_texture;
uniform vec2 u_repeat_v;
uniform sampler2D texture;
uniform mat4 model_matrix;
uniform vec3 u_color;
varying vec2 v_texcoord;
varying vec3 v_normal;
varying vec3 v_view_normal;
varying vec3 v_frag_pos;

//! [0]
void main()
{
    if(u_use_texture == 1)
    {
        vec2 dv[9];
        dv[0] = vec2(0.0,0.0);
        dv[1] = vec2(1.0,0.0);
        dv[2] = vec2(0.0,1.0);
        dv[3] = vec2(-1.0,0.0);
        dv[4] = vec2(0.0,-1.0);
        dv[5] = vec2(1.0,1.0);
        dv[6] = vec2(-1.0,1.0);
        dv[7] = vec2(1.0,-1.0);
        dv[8] = vec2(-1.0,-1.0);
        vec4 color = texture2D(texture, v_texcoord * u_repeat_v);
        for(int i=1; i < 9; i++)
        {
            color = color+ texture2D(texture,v_texcoord * u_repeat_v + dv[i] * vec2(1.0/512.0));
        }
        color = color / 9.0;
        gl_FragColor = color;

    }
    else
    {
        gl_FragColor = vec4(u_color *(abs(dot(normalize(v_view_normal),normalize(v_frag_pos)))+vec3(0.2)),1.0);
    }

}
//! [0]

