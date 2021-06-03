#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D texture;
uniform float u_width;
uniform float u_height;
uniform float texture_width;
uniform float texture_height;
uniform int u_state;
uniform float pack_range;
uniform int func_type;
uniform int val_type;
uniform float valid_min;
uniform float valid_max;
varying vec2 v_texcoord;
varying vec3 v_pos;


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

float getValue(vec2 texcoord)
{
    if(val_type < 1)
    {
        return texture2D(texture,texcoord).r;
    }
    else
    {
        return float(unpackInt(texture2D(texture,texcoord)));
//        return (unpack(texture2D(texture,texcoord))) * pack_range;
    }
}
bool isValid(float val)
{
    return (val >= valid_min && val <= valid_max);
}
vec4 packValue(float val, int vtype)
{
    if(vtype == 0)
    {
        val = val > 1.0 ?1.0:val;
        return vec4(val,0.0,0.0,1.0);
    }
    else
    {
        return packInt((floor(val)));
//        return pack(val / pack_range);
    }
}

vec4 getColor()
{
    vec2 dv = vec2(1.0/texture_width, 1.0/texture_width);
    vec2 tex = floor(v_texcoord * vec2(u_width, u_height)) * 2.0 * dv;
    if(func_type == 0 && val_type > 0)
    {
        vec4 resV = texture2D(texture, tex);
        resV = addPackInt(resV,texture2D(texture, tex+dv*vec2(1.0,0.0)));
        resV = addPackInt(resV,texture2D(texture, tex+dv*vec2(0.0,1.0)));
        resV = addPackInt(resV,texture2D(texture, tex+dv*vec2(1.0,1.0)));
        return resV;
    }
    float ans[4];
    ans[0] = getValue(tex);
    ans[1] = getValue(tex+dv*vec2(1.0,0.0));
    ans[2] = getValue(tex+dv*vec2(0.0,1.0));
    ans[3] = getValue(tex+dv*vec2(1.0,1.0));

    float res = 0.0;
    if(func_type == 0 || func_type == 2) // sum
    {
        res = ans[0] + ans[1] + ans[2] + ans[3];
    }
    else if(func_type == 1  )
    {
        res = ans[0];
        res = max(res,ans[1]);
        res = max(res,ans[2]);
        res = max(res,ans[3]);
    }
    return packValue(res,val_type);
}
//! [0]
void main()
{
    if(u_state < 1)
    {
        vec2 tex = (v_pos.xy + 1.0) * 0.5;
        float x = floor(u_width * tex.x);
        float y = floor(u_height * tex.y);

        x = x / texture_width;
        y = y / texture_height;

        if(x > 1.0 || y > 1.0)
        {
             gl_FragColor = packValue(0.0,1);
        }
        else
        {
            if(func_type == 2)
            {
               bool flag = isValid(getValue(vec2(x,y)));
               if(flag)
               {
                   gl_FragColor = packValue(1.,1);
               }
               else
               {
                   gl_FragColor = packValue(0.0,1);
               }
            }
            else
            {
                gl_FragColor = texture2D(texture, vec2(x,y));
            }
        }

    }
    else
    {
        gl_FragColor = getColor();
    }
}
//! [0]

