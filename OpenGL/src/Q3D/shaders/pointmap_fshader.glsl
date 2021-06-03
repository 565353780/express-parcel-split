#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;
varying vec4 v_color;
varying vec2 v_texcoord;
varying vec3 v_pos;
varying vec3 v_frag_pos;
uniform int u_width;
uniform int u_height;
uniform float pack_range;
uniform sampler2D z_map;
uniform sampler2D x_map;
uniform sampler2D y_map;
uniform sampler2D nor_map;
uniform sampler2D invalid_map;
uniform int has_normal;
uniform int need_smooth;
uniform int kernel_size;
uniform int use_point_map;
uniform int u_state;
uniform mat4 model_matrix;
uniform float u_max_depth;
uniform float u_min_depth;
uniform int u_map_type;
const float PI = 3.141592653589793;


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

vec3 getVertex(vec2 texcoord)
{
    return (vec3(unpack(texture2D(x_map,texcoord)),
                unpack(texture2D(y_map,texcoord)),
                unpack(texture2D(z_map,texcoord))) * 2.0 - 1.0) * pack_range;
}
vec3 getNormal()
{
    float dx = 1.0 / float(u_width);
    float dy = 1.0 / float(u_height);

    vec3 v0 = getVertex(v_texcoord);
    vec3 v1 = getVertex(v_texcoord + vec2(dx,0)) - v0;
    vec3 v2 = getVertex(v_texcoord + vec2(0,dy)) - v0;

    vec3 nor = cross(v1,v2);
    return normalize(nor);
}
float gauss(float x, float sigma){
    return exp(-(x*x)/(2.0*sigma*sigma)) / (2.0 * PI * sigma * sigma);
}
vec3 getNormalAtMap(vec2 texcoord)
{
    return (texture2D(nor_map,texcoord).xyz -0.5) * 2.0;
}
vec3 getSmoothNormal()
{
    float dx = 1.0 / float(u_width);
    float dy = 1.0 / float(u_height);
    int kernel_size = 3;
    vec3 nor =  getNormalAtMap(v_texcoord);
    vec3 nor0 = vec3(0.0,0.0,0.0);
    for(int i=-kernel_size; i <= kernel_size; i++)
    {
        for(int j=-kernel_size; j <= kernel_size; j++)
        {
            vec2 dv = vec2(float(i),float(j));
            vec3 n1 = getNormalAtMap(v_texcoord+dv*vec2(dx,dy));
            float gw = gauss(length(dv),0.8);
            float bw = gauss(length(n1-nor),0.45);
            nor0 += getNormalAtMap(v_texcoord+dv*vec2(dx,dy)) * gw * bw;
        }
    }
    return normalize(nor0);
}
bool isValid(vec2 texcoord)
{
   return (texture2D(invalid_map,texcoord).r < 0.5);
}



vec4 packInt(const in int val)
{
    vec4 v;

    v.x = floor(mod(float(val), 256.));
    v.y = floor(mod(float(val) / 256.0 , 256.0));
    v.z = floor(mod(float(val) / 256. / 256. , 256.));
    v.w = 255.0 - floor(mod(float(val) / 256. / 256. / 256., 256.));
    v /= 255.0;
    return v;
}

int unpackInt(const in vec4 v)
{
    int sum = 0;
    vec4 v1 = floor(v * 255.0);
    sum += int(v1.x);
    sum += int(v1.y) * 256;
    sum += int(v1.z) * 256 * 256;
    sum += int(255.0 - v1.w) * 256 * 256 * 256;
    return sum;
}
void main()
{
    if(u_state == 1)
    {
        vec3 tp = getVertex(v_texcoord);
        if(tp.z < -999.)
        {
            gl_FragColor = vec4(1.,0.,0.,1.);
        }
        else
        {
            gl_FragColor = vec4(0.0,0.0,0.0,1.0);
        }
        return;
    }
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

        float px = floor(v_texcoord.x * float(u_width))  / float(u_width);
        float py = floor(v_texcoord.y * float(u_height)) / float(u_height);
        vec2 tex = vec2(px,py);
        for(int i=0; i < 9; i++)
        {
            vec3 tp = getVertex(tex + dv[i] * vec2(1.0/float(u_width),1.0/float(u_height)));
            if(tp.z < -999.0)
            {
                discard;
                return;
            }
        }

    }

    if(u_state == 5)
    {
        vec4 tpos = model_matrix * vec4(v_pos,1.0);
//        gl_FragColor = pack((tpos.z / pack_range + 1.0)*0.5);
        gl_FragColor = packInt(int(v_pos.z * 100.0));
        return;
    }
    if(u_state == 7)
    {
        if(u_map_type == 0)
        {
            gl_FragColor =  pack((v_pos.x / pack_range + 1.0)*0.5);
        }
        else if(u_map_type == 1)
        {
            gl_FragColor =  pack((v_pos.y / pack_range + 1.0)*0.5);
        }
        else if(u_map_type == 2)
        {
            gl_FragColor =  pack((v_pos.z / pack_range + 1.0)*0.5);
        }
        return;
    }
    if(has_normal>=1)
    {
        if(u_state == 3)
        {
            if(!isValid(v_texcoord))
            {
//                gl_FragColor = vec4(1.0,0.0,0.0,1.0);
//                return;
            }
            vec3 nor0 = getNormalAtMap(v_texcoord);
            vec2 fpos = v_frag_pos.xy / v_frag_pos.z *0.5 + 0.5;
            fpos.x = 1.0 - fpos.x;
            vec3 nor1 = getNormalAtMap(fpos);
//            gl_FragColor = vec4((nor1*0.5+0.5),0.5);
            vec3 pos0 = getVertex(v_texcoord);
            vec3 pos1 = getVertex(fpos);
            float length = 20.0 -length(pos0 - pos1);
            if(length < 0.0) length = 0.0;
            gl_FragColor = packInt(int(length));
            return;
        }
        else if(u_state == 4)
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

            float px = floor(v_texcoord.x * float(u_width))  / float(u_width);
            float py = floor(v_texcoord.y * float(u_height)) / float(u_height);
            vec3 nor0 = getNormalAtMap(v_texcoord );
            vec3 v0 = getVertex(v_texcoord);
            if(v0.z > u_max_depth || v0.z < u_min_depth)
            {
                gl_FragColor = vec4(1.0,0.,0.,1.0);
                return;
            }
            for(int i=0; i < 9; i++)
            {
                vec3 nor1 = getNormalAtMap(v_texcoord +  dv[i] * vec2(1.0/float(u_width),1.0/float(u_height)));
                if(dot(nor0,nor1) < 0.9)
                {
                    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
                    return;
                }



                if((nor1.z < 0.8 || nor0.z <0.8) && dot(normalize(nor1.xy),normalize(nor0.xy)) < -0.7)
                {
                    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
                    return;
                }


                vec3 v1 = getVertex(v_texcoord +  dv[i] * vec2(1.0/float(u_width),1.0/float(u_height)));
                if(abs(v0.z - v1.z) > 8.)
                {
                    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
                    return;
                }

            }
            gl_FragColor = vec4(nor0*0.5+0.5,1.0);
            return;
        }
        if(u_state == 6)
        {
            vec3 nor0 = getNormalAtMap(v_texcoord);
            gl_FragColor = vec4(nor0*0.5+0.5,1.0);

            return;
        }
        gl_FragColor = vec4(getSmoothNormal()*0.5+0.5,1.0);
    }else{
        gl_FragColor =vec4(getNormal()*0.5+0.5,1.0);
    }
}
//! [0]

