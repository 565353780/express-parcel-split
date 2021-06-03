#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;
uniform sampler2D z_map;
uniform sampler2D nor_map;
uniform mat4 model_matrix;
uniform int u_state;
varying vec2 v_texcoord;
varying vec3 v_normal;
varying vec3 v_view_normal;
varying vec4 v_frag_pos;
varying vec3 v_pos;
uniform float pack_range;
uniform float u_mscore;
uniform float u_trunc;
vec4 pack(const in float depth)
{
    const vec4 bit_shift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const vec4 bit_mask  = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return res.wzyx;
}

float unpack(const in vec4 rgba_depth)
{
    const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth.wzyx, bit_shift);
    return depth;
}

vec4 packInt(const in float val)
{
    vec4 v;

    v.x = floor(mod(float(val), 256.));
    v.y = floor(mod(float(val) / 256.0 , 256.0));
    v.z = floor(mod(float(val) / 256. / 256. , 256.));
    v.w = 255.0 - floor(mod(float(val) / 256. / 256. / 256., 256.));
    v /= 255.0;
    return v;
}
vec4 addPackInt(vec4 v0,  vec4 v1)
{
    v0 =floor(v0* 255.0);
    v1 =floor(v1 * 255.0);
    v0.w = 255.0 - v0.w;
    v1.w = 255.0 - v1.w;

    vec4 ans;
    ans.x = v0.x + v1.x;

    ans.y = floor(ans.x / 256.0) + v0.y + v1.y;
    ans.x = floor(mod(ans.x, 256.0));

    ans.z = floor(ans.y / 256.0) + v0.z + v1.z;
    ans.y = floor(mod(ans.y, 256.0));

    ans.w = floor(ans.z / 256.0) + v0.w + v1.w;
    ans.z = floor(mod(ans.z, 256.0));

    ans.w = 255.0 - ans.w;
    ans /= 255.0;
    return ans;
}
float unpackInt(const in vec4 v)
{
    float sum = 0.0;
    vec4 v1 = floor(v * 255.0);
    sum += (v1.x);
    sum += (v1.y) * 256.;
    sum += (v1.z) * 256. * 256.;
    sum += (255.0 - v1.w) * 256. * 256. * 256.;
    return sum;
}

//! [0]
void main()
{

    // Set fragment color from texture
//    gl_FragColor = vec4(vec3(1.0,0.8,0.2) *(dot(v_view_normal,normalize(v_frag_pos))),1.0);
//    gl_FragColor = vec4(vec3(v_view_normal + 1.0) * 0.5,1.0);
//    gl_FragColor = vec4(texture2D(nor_map,fpos).xyz,1.0);

    vec2 fpos = v_frag_pos.xy / v_frag_pos.z *0.5 + 0.5;

    if(u_state != 2)
    {

        vec3 nor0 = texture2D(nor_map,fpos).xyz * 2.0 - 1.0;
        vec3 nor1 = v_view_normal;
        nor1.y = - nor1.y;
        vec2 dn = normalize(nor0.xy);
        float fitscore = dot(nor0,nor1);
        float z0 = unpackInt(texture2D(z_map,fpos)) / 100.0;
        float mscore = u_mscore;

        vec3 tpos =  v_pos;
        float z1 = tpos.z - z0;
        if(abs(z1)>u_trunc) z1 =mscore;
        z1 =(z1*z1);


        float length = mscore - z1;
        if(length < 0.0) length = 0.0;
        length = length * fitscore * fitscore * fitscore;
        if(length < 0.0) length = 0.0;

//        gl_FragColor =texture2D(z_map,fpos);// vec4(vec3(abs(z0) / 1000.0),1.0);
        if(u_state == 0)
        {
//             gl_FragColor =texture2D(z_map,fpos);
            gl_FragColor =packInt((v_pos.z)*100.0);
//            gl_FragColor =packInt(fitscore*100.0);
        }
        else
        {
            if(u_state == 3)
            {
                gl_FragColor =  packInt(abs(mscore));
            }
            else
            {
                gl_FragColor =  packInt(abs(length));
            }
        }

    }
    else
    {
//         gl_FragColor =texture2D(nor_map,fpos);
        vec3 nor = v_view_normal;
        nor.y = - nor.y;
        vec3 nor0 = texture2D(nor_map,fpos).xyz * 2.0 - 1.0;
//        gl_FragColor =  vec4( (dot(normalize(nor0.xy),normalize(nor.xy))));
//        gl_FragColor = vec4(vec3(nor + 1.0) * 0.5,0.5);
        gl_FragColor = vec4(vec3(1.0,0.8,0.2) *(dot(v_view_normal,normalize(v_frag_pos.xyz))),1.0);
//        gl_FragColor = pack((v_pos.z / pack_range + 1.0)*0.5);




    }

}
//! [0]

