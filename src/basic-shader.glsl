
#if defined(VS_BUILD)

out vec2 texCoord0;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 offset;

layout (std140) uniform ShaderData {
	vec3 cameraPosition;
};

void main() {
	gl_Position = vec4(2 * position + offset, 0.0, 1.0);
	texCoord0 = position;
}

#elif defined(FS_BUILD)

vec3 catmullRom( vec3 p0, vec3 p1, vec3 p2, vec3 p3, float weight ) {
    float weight2 = weight * weight;
    return 0.5 * (
        p0 * weight * ( ( 2.0 - weight ) * weight - 1.0 ) +
        p1 * ( weight2 * ( 3.0 * weight - 5.0 ) + 2.0 ) +
        p2 * weight * ( ( 4.0 - 3.0 * weight ) * weight + 1.0 ) +
        p3 * ( weight - 1.0 ) * weight2 );
}

// Performs a horizontal catmulrom operation at a given V value.
vec3 textureCubicU( sampler2D samp, vec2 uv00, float texel, float offsetV, float frac ) {
    return catmullRom(
        texture2D( samp, uv00 + vec2( -texel, offsetV ), 0.0 ).rgb,
        texture2D( samp, uv00 + vec2( 0.0, offsetV ), 0.0 ).rgb,
        texture2D( samp, uv00 + vec2( texel, offsetV ), 0.0 ).rgb,
        texture2D( samp, uv00 + vec2( texel * 2.0, offsetV ), 0.0 ).rgb,
    frac );
}

// Samples a texture using a bicubic sampling algorithm. This essentially queries neighbouring
// pixels to get an average value.
vec3 textureBicubic( sampler2D samp, vec2 uv00, vec2 texel, vec2 frac ) {
    return catmullRom(
        textureCubicU( samp, uv00, texel.x, -texel.y, frac.x ),
        textureCubicU( samp, uv00, texel.x, 0.0, frac.x ),
        textureCubicU( samp, uv00, texel.x, texel.y, frac.x ),
        textureCubicU( samp, uv00, texel.x, texel.y * 2.0, frac.x ),
    frac.y );
}

in vec2 texCoord0;

uniform sampler2D diffuse;

out vec4 outColor;

void main() {
	vec2 uv = texCoord0;
	vec2 size = vec2(256.0);
	
	float smoothness = 1.0;
	size /= smoothness;

	vec2 texel = vec2(1.0 / size);
	vec2 uv00 = floor(uv * size) / size;
	vec2 frac = vec2(uv - uv00) * size;

	vec3 height = textureBicubic(diffuse, uv00, texel, frac);

	outColor = vec4(height, 1.0);
}

#endif
