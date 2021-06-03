#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_color;
attribute vec2 a_texcoord;

varying vec2 v_texcoord;
varying vec4 v_color;
varying vec3 v_pos;
varying vec3 v_frag_pos;

uniform int use_point_map;
uniform sampler2D z_map;
uniform sampler2D x_map;
uniform sampler2D y_map;
uniform float pack_range;
uniform int u_width;
uniform int u_height;
vec4 pack(const in float depth)
{
    const vec4 bit_shift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const vec4 bit_mask  = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return res.abgr;
}

float unpack(const in vec4 rgba_depth)
{
    const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth.abgr, bit_shift);
    return depth;
}

vec3 getVertex(vec2 texcoord)
{
    return (vec3(unpack(texture2D(x_map,texcoord)),
                unpack(texture2D(y_map,texcoord)),
                unpack(texture2D(z_map,texcoord))) * 2.0 - 1.0) * pack_range;
}

void main()
{
    // Calculate vertex position in screen space

    if(use_point_map > 1)
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

        for(int i=0; i < 9; i++)
        {
            v_pos = getVertex(a_texcoord + dv[i] * vec2(1.0 / float(u_width), 1.0 / float(u_height)));
            if(v_pos.z > -999.0) break;
        }

        gl_Position = mvp_matrix * vec4(v_pos,1);
        v_texcoord = a_texcoord;
        v_color = a_color;
        v_frag_pos = gl_Position.xyz / gl_Position.w;

    }
    else
    {
        gl_Position = mvp_matrix * vec4(a_position,1);
        // Pass texture coordinate to fragment shader
        // Value will be automatically interpolated to fragments inside polygon faces
        v_texcoord = a_texcoord;
        v_color = a_color;
        v_pos = a_position;
        v_frag_pos = gl_Position.xyz / gl_Position.w;
    }

}
//! [0]
