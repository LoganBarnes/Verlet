#version 410 core

// The ocean rendering is taken from https://www.shadertoy.com/view/Ms2SD1
// "Seascape" by Alexander Alekseev aka TDM - 2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

in vec3 pos;

out vec4 fragColor;

uniform vec2 viewport;
uniform mat4 filmToWorld;
uniform vec4 camEye;
uniform samplerCube envMap;
uniform float time;


uniform int NUM_TRIS = 2;
uniform int NUM_OBJECTS = 0;
uniform int NUM_TRANSPARENTS = 0;
uniform int NUM_LIGHTS = 1;

uniform mat4 invs[20];
uniform vec4 colors[20];
uniform int types[20];

uniform mat4 invsT[10];
uniform vec4 colorsT[10];
uniform int typesT[10];

// to be uniforms or blocks
vec3 lightPos[10];

// Constants
const float N_w = 1.336; // water
const float N_a = 1.0001; // air
//const float N_g = 1.513; // glass
const float N_g = 2.0001;

const float WATER_HEIGHT = 0.0;
const vec3 LIGHT_DIR = vec3(-0.89, -0.41, -0.2);
const vec3 LIGHT_COLOR = vec3(1, 1, 1);
const float SHINE = 64.0;

const float INF = 10000.0;
const float EPS = 0.00001;

const int TRI_SIZE = 4096;

layout (std140) uniform triBlock {
    vec4 tris[TRI_SIZE];
};


///////////////////////////////////////////////////////////////////
//                         START OF OCEAN                        //
//                         RENDERING CODE                        //
///////////////////////////////////////////////////////////////////

const int NUM_STEPS = 8;
const float PI    = 3.1415;
const float EPSILON = 1e-3;
float EPSILON_NRM = 0.1 / viewport.x;

// sea
const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.1;
const float SEA_CHOPPY = 2.0;
const float SEA_SPEED = 0.5;
const float SEA_FREQ = 0.6;
const vec3 SEA_BASE = vec3(0.1,0.19,0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
float SEA_TIME = time * SEA_SPEED;
mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

// math
mat3 fromEuler(vec3 ang) {
  vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
  m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
  m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
  return m;
}
float hash( vec2 p ) {
  float h = dot(p,vec2(127.1,311.7)); 
    return fract(sin(h)*43758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );  
  vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

// lighting
float diffuse(vec3 n,vec3 l,float p) {
    return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(vec3 n,vec3 l,vec3 e,float s) {    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sky
vec3 getSkyColor(vec3 e) {
    e.y = max(e.y,0.0);
    vec3 ret;
    ret.x = pow(1.0-e.y,2.0);
    ret.y = 1.0-e.y;
    ret.z = 0.6+(1.0-e.y)*0.4;
    return ret;
}

// sea
float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {        
      d = sea_octave((uv+SEA_TIME)*freq,choppy);
      d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;// + sin(p.x * .1 + time) * .1;
      uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float map_detailed(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {        
      d = sea_octave((uv+SEA_TIME)*freq,choppy);
      d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;// + sin(p.x * .1 + time) * .1; // how to add waves
      uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {  
    float fresnel = 1.0 - max(dot(n,-eye),0.0);
    fresnel = pow(fresnel,3.0) * 0.65;
        
    vec3 reflected = getSkyColor(reflect(eye,n));    
    vec3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.12; 
    
    vec3 color = mix(refracted,reflected,fresnel);
    
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    color += vec3(specular(n,l,eye,60.0));
    
    return color;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
    vec3 n;
    n.y = map_detailed(p);    
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {  
    float tm = 0.0;
    float tx = 1000.0;    
    float hx = map(ori + dir * tx);
    if(hx > 0.0) return tx;   
    float hm = map(ori + dir * tm);    
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));                   
        p = ori + dir * tmid;                   
      float hmid = map(p);
    if(hmid < 0.0) {
          tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}


vec3 oceanStuff(in vec3 ori, in vec3 dir)
{
    // tracing
    vec3 p;
    heightMapTracing(ori,dir,p);
    vec3 dist = p - ori;
    vec3 n = getNormal(p, dot(dist,dist) * EPSILON_NRM);
    vec3 light = normalize(-LIGHT_DIR);

    // color
    vec3 color = mix(
        getSkyColor(dir),
        getSeaColor(p,n,light,dir,dist),
        pow(smoothstep(0.0,-0.05,dir.y),0.3));

    // post
    return pow(color,vec3(0.75));
}


///////////////////////////////////////////////////////////////////
//                          END OF OCEAN                         //
//                         RENDERING CODE                        //
///////////////////////////////////////////////////////////////////


////////////////////// MY STUFF ///////////////////////////

//////////////////////////////////////////////
//                PHYSICS                   //
//////////////////////////////////////////////

float fresnel(in vec3 i, in vec3 t, in vec3 norm, in float n1, in float n2)
{
    float cosi = dot(-i, norm);
    float cost = dot(t, -norm);
    float rp = (n2*cosi - n1*cost) / (n2*cosi + n1*cost);
    float rs = (n1*cosi - n2*cost) / (n1*cosi + n2*cost);
    return (rp*rp + rs*rs) * .5;
}


///////////////////////////////////////////
//              INTERSECTIONS            //
///////////////////////////////////////////

int findT(in float a, in float b, in float c, out float t1, out float t2)
{
    // intersections, intersectDist1, intersectDist2
    vec3 t = vec3(0.0, INF, INF);

    if (abs(a) < EPS)
    {
        t1 = -c / b;
        return 1;
    }

    float disc = b * b - 4.0 * a * c;

    // one solution
    if (abs(disc) < EPS)
    {
        t1 = -b / (2.0 * a);
        t.x = 0.0;
        return 1;
    }

    // no solutions
    if (disc < 0.0)
        return 0;

    // two solutions (disc > 0)
    t1 = (-b + sqrt(disc)) / (2.0 * a);
    t2 = (-b - sqrt(disc)) / (2.0 * a);
    return 2;
}

///////////////////CYLINDER////////////////////

vec4 intersectCylinder(in vec4 p, in vec4 d)
{
    vec4 n = vec4(0, 0, 0, INF);

//    float horiz = p.x * p.x + p.z * p.z;
//    float vert = abs(p.y);

    // if p is in the shape don't register an intersection
//    if ((EQ(horiz, .25) || horiz < .25) && (EQ(vert, 0.5) || vert < 0.5))
//        return n;

    float t1 = INF;
    float t2 = INF;
    vec4 v;

    float a = d.x * d.x + d.z * d.z;
    float b = 2.0 * p.x * d.x + 2.0 * p.z * d.z;
    float c = p.x * p.x + p.z * p.z - 0.25;

    int tees = findT(a, b, c, t1, t2);

    if (tees > 0) {
        v = p + t1 * d;
        if (v.y > 0.5 || v.y < -0.5 || t1 < 0)
            t1 = INF;
        if (t1 < n.w) {
            n = vec4(v.x, 0, v.z, t1);
        }
        if (tees > 1) {
            v = p + t2 * d;
            if (v.y > 0.5 || v.y < -0.5 || t2 < 0)
                t2 = INF;
            if (t2 < n.w) {
                n = vec4(v.x, 0, v.z, t2);
            }
        }
    }

    float t3 = (0.5 - p.y) / d.y;
    v = p + t3 * d;

    if (v.x * v.x + v.z * v.z > 0.25 || t3 < 0.0)
        t3 = INF;
    if (t3 < n.w)
        n = vec4(0, 1, 0, t3);

    float t4 = (-0.5 - p.y) / d.y;
    v = p + t4 * d;

    if (v.x * v.x + v.z * v.z > 0.25 || t4 < 0.0)
        t4 = INF;
    if (t4 < n.w) {
        n = vec4(0, -1, 0, t4);
    }

    return n;
}


////////////////////SPHERE/////////////////////

// assumes radius size .5f
vec4 intersectSphere(in vec4 p, in vec4 d)
{
    vec4 n = vec4(0.0, 0.0, 0.0, INF);

    vec4 v;
    float t1, t2;
    float a = d.x * d.x + d.y * d.y + d.z * d.z;
    float b = 2.0 * p.x * d.x + 2.0 * p.y * d.y + 2.0 * p.z * d.z;
    float c = p.x * p.x + p.y * p.y + p.z * p.z - .25;
    int tees = findT(a, b, c, t1, t2);

    if (tees > 0)
    {
        v = p + d * t1;
        if (t1 < 0.0)
            t1 = INF;
        if (t1 < n.w)
        {
            v.w = t1;
            n = v;
        }
        if (tees > 1)
        {
            v = p + d * t2;
            if (t2 < 0.0)
                t2 = INF;
            if (t2 < n.w)
            {
                v.w = t2;
                n = v;
            }
        }
    }
    return n;
}

/////////////////////PLANE//////////////////////

vec4 intersectPlaneXZ(in vec4 p, in vec4 d)
{
    vec4 n = vec4(0, 0, 0, INF);

    n.w = WATER_HEIGHT - p.y / d.y;
    vec3 point = p.xyz + d.xyz * n.w;
    float radius = 700.0;
    if (n.w < EPS || point.x * point.x + point.z * point.z > radius * radius)
        n.w = INF;

    if (n.w < INF)
    {
        n.xyz = vec3(0, 1, 0);
    }
    return n;
}

/////////////////////ALL_OBJECTS//////////////////////

vec4 intersectObjects(in int exception, in vec4 p, in vec4 d, out int colorIndex)
{
    vec4 bestN = vec4(0, 0, 0, INF);
    vec4 n;

    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
        if (i == exception)
            continue;

        vec4 p_shape = invs[i] * p;
        vec4 d_shape = invs[i] * d;
        if (types[i] == 3)
            n = intersectCylinder(p_shape, d_shape);
        if (types[i] == 4)
            n = intersectSphere(p_shape, d_shape);
        if (n.w < bestN.w)
        {
            bestN.w = n.w;
            bestN.xyz = normalize(mat3(transpose(invs[i])) * n.xyz);
            colorIndex = i;
        }
    }
    return bestN;
}


vec4 intersectWorld(in int exception, in vec4 p, in vec4 d, out int colorIndex)
{
    vec4 bestN = vec4(0, 0, 0, INF);
    vec4 n;

    n = intersectObjects(exception, p, d, colorIndex);
    if (n.w < bestN.w)
        bestN = n;

    n = intersectPlaneXZ(p, d);
    if (n.w < bestN.w && d.y < 0.0)
    {
        bestN = n;
        colorIndex = -1;
    }
    return bestN;
}

//////////////////TRANSPARENT_OBJECTS////////////////////

vec4 intersectTransparents(in vec4 p, in vec4 d, out float t)
{
    vec4 color = vec4(0);
    float tempT;
    t = INF;

    for (int i = 0; i < NUM_TRANSPARENTS; ++i)
    {
        vec4 p_shape = invsT[i] * p;
        vec4 d_shape = invsT[i] * d;
        if (typesT[i] == 3)
            tempT = intersectCylinder(p_shape, d_shape).w;
        if (typesT[i] == 4)
            tempT = intersectSphere(p_shape, d_shape).w;
        if (tempT < t)
        {
            if (t == INF)
                color = colorsT[i];
            else
                color.xyz = mix(color.xyz, colorsT[i].xyz, color.w);
            t = tempT;
        }
    }

    return color;
}


///////////////////////////////////////////
//                 COLORS                //
///////////////////////////////////////////

vec3 getTexturedSky(vec3 dir)
{
    vec3 color = texture(envMap, dir.xyz).xyz;
    return color;
    float fade = pow(1.0 - max(dir.y, 0), 50.0);
    color = color + fade * (vec3(1) - color);
    return color;
}

vec3 calcObjectColorSolid(in int index, in vec3 point, in vec3 normal, in vec3 eye)
{
    vec3 color = colors[index].xyz * 0.075;
//    vec3 vertexToLight = normalize(LIGHT_POS - point);

    int colorIndex;
    vec3 bumpPoint = point + normal * EPS;
    if (intersectObjects(index, vec4(bumpPoint, 1), vec4(normalize(-LIGHT_DIR), 0), colorIndex).w < INF)
        return color;

    vec3 vertexToLight = normalize(-LIGHT_DIR);

    // Add diffuse component
    float diffuseIntensity = max(0.0, dot(vertexToLight, normal));
    color += max(vec3(0), LIGHT_COLOR * colors[index].xyz * diffuseIntensity);

    // Add specular component
    vec3 lightReflection = normalize(reflect(-vertexToLight, normal));
    vec3 eyeDirection = normalize(eye - point);
    float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), SHINE);
    color += max(vec3(0), texture(envMap, vertexToLight).xyz * specIntensity);

    return color;
}


vec3 calcWaterColor(in vec3 p, in vec3 norm, in vec3 eye)
{

    vec3 w_i = normalize(p - eye);
    vec3 ori = vec3(eye);

    // tracing
    vec3 point;
    heightMapTracing(ori,w_i,point);
    vec3 dist = point - ori;
    vec3 normal = getNormal(point, dot(dist,dist) * EPSILON_NRM);
//    vec3 light = normalize(vec3(0.0,1.0,0.8));
    vec3 light = normalize(-LIGHT_DIR);


    vec3 reflectVec = reflect(w_i, normal);
    vec3 refractVec = refract(w_i, normal, N_a / N_w);

    vec3 reflection;
    int colorIndex;
    vec3 bumpPoint = point + normal * 0.001;
    if (intersectObjects(-1, vec4(bumpPoint, 1), vec4(normalize(-LIGHT_DIR), 0), colorIndex).w == INF)
    {
        vec4 n = intersectObjects(-1, vec4(bumpPoint, 1), vec4(reflectVec, 0), colorIndex);

        if (n.w < INF)
            reflection = calcObjectColorSolid(colorIndex, bumpPoint + reflectVec * n.w, n.xyz, point);
        else
            reflection = texture(envMap, reflectVec).xyz;
    }
    else
        reflection = vec3(0);

    vec3 refraction = texture(envMap, refractVec).xyz * vec3(.5, .7, .5);

    float F = fresnel(w_i, refractVec, normal, N_a, N_w);

    return reflection * F + (1.0 - F) * refraction;
}

vec3 calcObjectColor(in int index, in vec3 point, in vec3 normal, in vec3 eye)
{
    vec3 dist = point - eye;
    vec3 w_i = normalize(dist);
    vec3 reflectVec = reflect(w_i, normal);
    vec3 refractVec = refract(w_i, normal, N_a / N_g);

    int colorIndex;
    vec3 bumpPoint = point + normal * 0.001;
    vec4 n = intersectWorld(index, vec4(bumpPoint, 1), vec4(reflectVec, 0), colorIndex);

    vec3 reflection = vec3(0,1,0);
    if (n.w < INF)
    {
        vec3 newPoint = bumpPoint.xyz + reflectVec.xyz * n.w;

        if (colorIndex == -1)
            // calc ocean color
            reflection = calcWaterColor(newPoint, n.xyz, eye.xyz);
        else
            // calc object colors
            reflection = calcObjectColorSolid(colorIndex, newPoint, n.xyz, eye.xyz);
    }
    else
        reflection = texture(envMap, reflectVec).xyz;

    vec3 refraction = calcObjectColorSolid(index, point, normal, eye);

    float F = fresnel(w_i, refractVec, normal, N_a, N_g);
    return reflection * F + (1.0 - F) * refraction;
}


vec3 raytrace(in vec4 p, in vec4 d)
{
    int colorIndex;
    float t;
    vec4 n = intersectWorld(-1, p, d, colorIndex);
    vec4 transColor = intersectTransparents(p, d, t);

    if (n.w < INF)
    {
        vec3 point = p.xyz + d.xyz * n.w;

        // calc ocean color
        if (colorIndex == -1)
        {
            if ((p + d * t).y < WATER_HEIGHT)
                return calcWaterColor(point, n.xyz, p.xyz);

            return mix(calcWaterColor(point, n.xyz, p.xyz), transColor.xyz, transColor.w);
        }

        // calc object colors
        if (t < n.w)
            return mix(calcObjectColor(colorIndex, point, n.xyz, p.xyz), transColor.xyz, transColor.w);

        return calcObjectColor(colorIndex, point, n.xyz, p.xyz);
    }

    vec3 color = getTexturedSky(d.xyz);

    color = mix(color,
                oceanStuff(vec3(p), vec3(d)),
                pow(smoothstep(0.0,-0.05,d.y),0.3));

    color = pow(color,vec3(0.75));


    return mix(color, transColor.xyz, transColor.w);
}


// main
void main() {

    vec4 farWorld = filmToWorld * vec4(pos, 1);
    vec4 dir = normalize(farWorld - camEye);
        
    vec3 color = raytrace(camEye, dir);
    fragColor = vec4(color, 1);
}
