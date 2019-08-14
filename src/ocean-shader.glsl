
#if defined(VS_BUILD)

out vec3 normal;

uniform sampler2D ocean;

layout(location = 0) in vec2 xyPos;
layout(location = 1) in vec4 adjacent;
layout(location = 2) in mat4 transform;

layout (std140) uniform ShaderData {
	vec4 corners[4];
};

// catmull works by specifying 4 control points p0, p1, p2, p3 and a weight. The function is used to calculate a point n between p1 and p2 based
// on the weight. The weight is normalized, so if it's a value of 0 then the return value will be p1 and if its 1 it will return p2. 
float catmullRom( float p0, float p1, float p2, float p3, float weight ) {
    float weight2 = weight * weight;
    return 0.5 * (
        p0 * weight * ( ( 2.0 - weight ) * weight - 1.0 ) +
        p1 * ( weight2 * ( 3.0 * weight - 5.0 ) + 2.0 ) +
        p2 * weight * ( ( 4.0 - 3.0 * weight ) * weight + 1.0 ) +
        p3 * ( weight - 1.0 ) * weight2 );
}

// Performs a horizontal catmulrom operation at a given V value.
float textureCubicU( sampler2D samp, vec2 uv00, float texel, float offsetV, float frac ) {
    return catmullRom(
        texture2D( samp, uv00 + vec2( -texel, offsetV ), 0.0 ).r,
        texture2D( samp, uv00 + vec2( 0.0, offsetV ), 0.0 ).r,
        texture2D( samp, uv00 + vec2( texel, offsetV ), 0.0 ).r,
        texture2D( samp, uv00 + vec2( texel * 2.0, offsetV ), 0.0 ).r,
    frac );
}

// Samples a texture using a bicubic sampling algorithm. This essentially queries neighbouring
// pixels to get an average value.
float textureBicubic( sampler2D samp, vec2 uv00, vec2 texel, vec2 frac ) {
    return catmullRom(
        textureCubicU( samp, uv00, texel.x, -texel.y, frac.x ),
        textureCubicU( samp, uv00, texel.x, 0.0, frac.x ),
        textureCubicU( samp, uv00, texel.x, texel.y, frac.x ),
        textureCubicU( samp, uv00, texel.x, texel.y * 2.0, frac.x ),
    frac.y );
}

float height(vec2 pos) {
	return 2 * texture2D(ocean, pos).y - 1
		+ 0.01 * texture2D(ocean, 100 * pos).y;
	
	/*vec2 heightSize = vec2(256.0, 256.0);
	heightSize /= 0.1;
	
	const vec2 heightUV = 0.01 * pos;
	const vec2 texel = vec2(1.0 / heightSize);
	const vec2 heightUV00 = floor(heightUV * heightSize) / heightSize;
	const vec2 frac = (heightUV - heightUV00) / heightSize;
	return 2 * textureBicubic(ocean, heightUV00, texel, frac) - 1;*/
}

vec4 getOceanPosition(vec2 pos) {
	const vec4 a = mix(corners[0], corners[2], pos.x);
	const vec4 b = mix(corners[1], corners[3], pos.x);

	vec4 o = mix(a, b, pos.y);
	o.y = height(o.xz / o.w * 0.01) * o.w;

	return o;
}

void main() {
	vec4 p0Raw = getOceanPosition(xyPos);
	vec4 p1Raw = getOceanPosition(xyPos + adjacent.xy);
	vec4 p2Raw = getOceanPosition(xyPos + adjacent.zw);

	vec3 p0 = p0Raw.xyz / p0Raw.w;
	vec3 p1 = p1Raw.xyz / p1Raw.w;
	vec3 p2 = p2Raw.xyz / p2Raw.w;

	normal = normalize(cross(p1 - p0, p2 - p0));

	gl_Position = transform * p0Raw;
}

#elif defined(FS_BUILD)

uniform sampler2D ocean;

in vec3 normal;

out vec4 outColor;

void main() {
	float diffuse = dot(normal, normalize(vec3(1, 1, 0)));
	outColor = vec4(vec3(diffuse), 1.0);
}

#endif
