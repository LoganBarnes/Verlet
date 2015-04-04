#version 410 core

in vec3 pos;

out vec4 fragColor;


uniform vec2 viewport;
uniform mat4 filmToWorld;
uniform vec4 camEye;
uniform samplerCube envMap;

uniform int NUM_TRIS = 2;
uniform int NUM_OBJECTS = 0;
uniform int NUM_LIGHTS = 1;

uniform mat4 invs[20];
uniform vec3 colors[20];
uniform int types[20];
// to be uniforms or blocks
vec3 lightPos[10];

// Constants
const float N_w = 1.336; // water
const float N_a = 1.0001; // air
//const float N_g = 1.513; // glass
const float N_g = 2.0001;

const float WATER_HEIGHT = 1.0;
const vec3 LIGHT_DIR = vec3(-1, -1.2, .4);
const vec3 LIGHT_COLOR = vec3(1, 1, 1);
const float SHINE = 64.0;

const float INF = 10000.0;
const float EPS = 0.00001;

const int TRI_SIZE = 4096;

//vec3 tris[10];
layout (std140) uniform triBlock {
    vec4 tris[TRI_SIZE];
//    vec3 trisz[TRI_SIZE];
};



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
    if (n.w < EPS)
        n.w = INF;

    if (n.w < INF)
    {
        // calc pos and normal for waves
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


///////////////////////////////////////////
//                 COLORS                //
///////////////////////////////////////////

vec3 calcObjectColorSolid(in int index, in vec3 point, in vec3 normal, in vec3 eye)
{
    vec3 color = colors[index] * 0.075;
//    vec3 vertexToLight = normalize(LIGHT_POS - point);

    int colorIndex;
    vec3 bumpPoint = point + normal * EPS;
    if (intersectObjects(index, vec4(bumpPoint, 1), vec4(normalize(-LIGHT_DIR), 0), colorIndex).w < INF)
        return color;

    vec3 vertexToLight = normalize(-LIGHT_DIR);

    // Add diffuse component
    float diffuseIntensity = max(0.0, dot(vertexToLight, normal));
    color += max(vec3(0), LIGHT_COLOR * colors[index] * diffuseIntensity);

    // Add specular component
    vec3 lightReflection = normalize(reflect(-vertexToLight, normal));
    vec3 eyeDirection = normalize(eye - point);
    float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), SHINE);
    color += max(vec3(0), texture(envMap, vertexToLight).xyz * specIntensity);

    return color;
}


vec3 calcWaterColor(in vec3 point, in vec3 normal, in vec3 eye)
{

    vec3 w_i = normalize(point - eye);
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
    vec3 w_i = normalize(point - eye);
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
//F = 0;
    return reflection * F + (1.0 - F) * refraction;
}


vec3 raytrace(in vec4 p, in vec4 d)
{
    int colorIndex;
    vec4 n = intersectWorld(-1, p, d, colorIndex);

    if (n.w < INF)
    {
        vec3 point = p.xyz + d.xyz * n.w;

        // calc ocean color
        if (colorIndex == -1)
            return calcWaterColor(point, n.xyz, p.xyz);

        // calc object colors
        return calcObjectColor(colorIndex, point, n.xyz, p.xyz);
    }

//    return vec3(0);
    return texture(envMap, d.xyz).xyz;
}




void main()
{

    // testing
//    tris[0] = vec3(0,1,-5);
//    tris[1] = vec3(-1,-1,-5);
//    tris[2] = vec3(1,-1,-3);

//    colors[0] = vec3(1,.5,0);
//    colors[1] = vec3(1,.5,0);
//    colors[2] = vec3(1,.5,0);

    // heh
//    invs[0] = mat4(1, 0, 0, 0,
//                   0, 1, 0, 0,
//                   0, 0, 1, 0,
//                   0,-3, 5, 1);

//    invs[1] = mat4(1, 0, 0, 0,
//                   0, 1, 0, 0,
//                   0, 0, 1, 0,
//                   0,-3, 4, 1);

//    invs[2] = mat4(1.5, 0, 0, 0,
//                   0, .5, 0, 0,
//                   0, 0, 1.5, 0,
//                   0,-2.5, 6.75, 1);
//    invs[1][3] = vec4(-LIGHT_POS, 1);
    // end testing

    vec4 farWorld = filmToWorld * vec4(pos, 1);
    vec4 dir = normalize(farWorld - camEye);

    vec3 color = raytrace(camEye, dir);
    fragColor = vec4(color, 1);
}
