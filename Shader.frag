#version 130
 
uniform vec2 u_resolution;
uniform vec3 u_offset;
uniform vec2 u_angle;
uniform vec3 u_pos;
uniform vec2 u_seed1;
uniform vec2 u_seed2;
uniform int u_samples;
uniform int u_refs;
uniform float u_max_dist;
uniform vec2 u_light;

uniform int u_sph_num;
uniform int u_box_num;
uniform int u_tri_num;

uniform vec4 u_sph_cord[100];
uniform vec3 u_box_cord[200];
uniform mat3 u_tri_cord[100];

uniform vec4 u_sph_col[100];
uniform vec4 u_box_col[100];
uniform vec4 u_tri_col[100];


vec3 light = normalize(vec3(0.0, u_light.x, u_light.y));

mat2 rot(float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c);
}

uvec4 R_STATE;

uint TausStep(uint z, int S1, int S2, int S3, uint M) {
    uint b = (((z << S1) ^ z) >> S2);
    return (((z & M) << S3) ^ b);
}

uint LCGStep(uint z, uint A, uint C) {
    return (A * z + C);
}

vec2 hash22(vec2 p) {
    p += u_seed1.x;
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx+33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

void set_random(vec2 uv) {
    vec2 uvRes = hash22(uv + 1.0) * u_resolution + u_resolution;
    R_STATE.x = uint(u_seed1.x + uvRes.x);
    R_STATE.y = uint(u_seed1.y + uvRes.x);
    R_STATE.z = uint(u_seed2.x + uvRes.y);
    R_STATE.w = uint(u_seed2.y + uvRes.y);
}

vec2 get_cord(vec2 cord) {
    vec2 ans = (cord - 0.5) * u_resolution / max(u_resolution.x, u_resolution.y) + u_offset.xy;
    set_random(ans);
    return ans;
}

vec3 get_dir(vec2 cord) {
    vec3 ans = normalize(vec3(u_offset.z, cord));
    ans.zx *= rot(-u_angle.y);
    ans.xy *= rot(u_angle.x);
    return ans;
}

float random() {
    R_STATE.x = TausStep(R_STATE.x, 13, 19, 12, uint(4294967294));
    R_STATE.y = TausStep(R_STATE.y, 2, 25, 4, uint(4294967288));
    R_STATE.z = TausStep(R_STATE.z, 3, 11, 17, uint(4294967280));
    R_STATE.w = LCGStep(R_STATE.w, uint(1664525), uint(1013904223));
    return 2.3283064365387e-10 * float((R_STATE.x ^ R_STATE.y ^ R_STATE.z ^ R_STATE.w));
}

float atan2(vec2 dir) {
    float angle = asin(dir.x) > 0 ? acos(dir.y) : -acos(dir.y);
    return angle;
}

vec3 randomOnSphere() {
    vec3 rand = vec3(random(), random(), random());
    float theta = rand.x * 2.0 * 3.14159265;
    float v = rand.y;
    float phi = acos(2.0 * v - 1.0);
    float r = pow(rand.z, 1.0 / 3.0);
    float x = r * sin(phi) * cos(theta);
    float y = r * sin(phi) * sin(theta);
    float z = r * cos(phi);
    return vec3(x, y, z);
}

vec2 sphIntersect(in vec3 ro, in vec3 rd, float ra) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - ra * ra;
    float h = b * b - c;
    if (h < 0.0) {
        return vec2(-1.0);
    }
    h = sqrt(h);
    return vec2(-b - h, -b + h);
}

vec2 boxIntersection(in vec3 ro, in vec3 rd, in vec3 rad, out vec3 oN)  {
    vec3 m = 1.0 / rd;
    vec3 n = m * ro;
    vec3 k = abs(m) * rad;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = min(min(t2.x, t2.y), t2.z);
    if (tN > tF || tF < 0.0) {
        return vec2(-1.0);
    }
    oN = -sign(rd) * step(t1.yzx, t1.xyz) * step(t1.zxy, t1.xyz);
    return vec2(tN, tF);
}

float plaIntersect(in vec3 ro, in vec3 rd, in vec4 p) {
    return -(dot(ro, p.xyz) + p.w) / dot(rd, p.xyz);
}

vec3 triIntersection(in vec3 ro, in vec3 rd, in vec3 v0, in vec3 v1, in vec3 v2) {
    vec3 v1v0 = v1 - v0;
    vec3 v2v0 = v2 - v0;
    vec3 rov0 = ro - v0;
    vec3 n = cross(v1v0, v2v0);
    vec3 q = cross(rov0, rd);
    float d = 1.0 / dot(rd, n);
    float u = d * dot(-q, v2v0);
    float v = d * dot(q, v1v0);
    float t = d * dot(-n, rov0);
    if (u < 0.0 || u > 1.0 || v < 0.0 || (u + v) > 1.0) {
        t = -1.0;
    }
    return vec3(t, u, v);
}

vec3 getSky(vec3 rd) {
    vec3 col = vec3(0.4, 0.6, 1.0);
    vec3 sun = vec3(0.95, 0.9, 1.0);
    sun *= max(0.0, pow(dot(rd, light), 128.0));
    col *= max(0.0, dot(light, vec3(0.0, 0.0, -1.0)));
    return clamp(sun + col * 0.01, 0.0, 1.0);
}

float[9] colliderIntersection(vec3 ro, vec3 rd) {
    vec4 col;
    vec2 minIt = vec2(u_max_dist);
    vec2 it;
    vec3 n;

    for (int i = 0; i < u_sph_num; i++) {
        it = sphIntersect(ro - u_sph_cord[i].xyz, rd, u_sph_cord[i].w);
        if (it.x > 0.0 && it.x < minIt.x) {
            minIt = it;
            vec3 itPos = ro + rd * it.x;
            n = normalize(itPos - u_sph_cord[i].xyz);
            col = u_sph_col[i];
        }
    }

    vec3 boxN;

    for (int i = 0; i < u_box_num; i++) {
        it = boxIntersection(ro - u_box_cord[2 * i], rd, u_box_cord[2 * i + 1], boxN);
        if (it.x > 0.0 && it.x < minIt.x) {
            minIt = it;
            n = boxN;
            col = u_box_col[i];
        }
    }

    for (int i = 0; i < u_tri_num; i++) {
        it = vec2(triIntersection(ro, rd, u_tri_cord[i][0], u_tri_cord[i][1], u_tri_cord[i][2]));
        if (it.x > 0.0 && it.x < minIt.x) {
            minIt = it;
            vec3 v0 = u_tri_cord[i][0] - u_tri_cord[i][1];
            vec3 v1 = u_tri_cord[i][2] - u_tri_cord[i][1];
            n = normalize(cross(v0, v1));
            col = u_tri_col[i];
        }
    }

    float result[9];

    result[0] = col.x;
    result[1] = col.y;
    result[2] = col.z;
    result[3] = col.w;

    result[4] = minIt.x;
    result[5] = minIt.y;

    result[6] = n.x;
    result[7] = n.y;
    result[8] = n.z;

    return result;
}

vec4 castRay(inout vec3 ro, inout vec3 rd) {
    vec4 col;
    vec2 minIt = vec2(u_max_dist);
    vec2 it;
    vec3 n;

    float data[9] = colliderIntersection(ro, rd);
    it = vec2(data[4], data[5]);
    if (it.x > 0.0 && it.x < minIt.x) {
        minIt = it;
        n = vec3(data[6], data[7], data[8]);
        col = vec4(data[0], data[1], data[2], data[3]);
    }

    vec3 planeNormal = vec3(0.0, 0.0, -1.0);
    it = vec2(plaIntersect(ro, rd, vec4(planeNormal, 1.0)));
    if (it.x > 0.0 && it.x < minIt.x) {
        minIt = it;
        n = planeNormal;
        col = vec4(0.1, 0.7, 0.1, 0.5);
    }

    if (minIt.x == u_max_dist) {
        return vec4(getSky(rd), 1000.0);
    }

    if (col.a == 1000.0) {
        return col;
    }

    vec3 reflected = reflect(rd, n);
    if (col.a < 0.0) {
        ro += rd * (minIt.y + 0.001);
        rd = refract(rd, n, -col.a);
        return col;
    }
    vec3 itPos = ro + rd * it.x;
    vec3 r = randomOnSphere();
    vec3 diffuse = normalize(r * dot(r, n));
    ro += rd * (minIt.x - 0.001);
    rd = mix(diffuse, reflected, col.a);
    return col;
}

vec3 traceRay(vec3 ro, vec3 rd) {
    vec3 col = vec3(1.0);
    for (int i = 0; i <= u_refs; i++) {
        vec4 refCol = castRay(ro, rd);
        col *= refCol.rgb;
        if (refCol.a == 1000.0) {
            break;
        }
    }
    return col;
}

void main() {
    vec2 cord = get_cord(gl_TexCoord[0].xy);
    vec3 color = vec3(0.0);
    for (int i = 0; i < u_samples; i++) {
        color += traceRay(u_pos, get_dir(cord));
    }
    color /= u_samples;

    float white = 20.0;
    color *= white * 8.0;
    color = (color * (1.0 + color / white / white)) / (1.0 + color);
    gl_FragColor = vec4(color, 1.0);
}
