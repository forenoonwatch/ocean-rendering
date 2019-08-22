#include "commong.glh"

varying vec2 texCoord0;

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
	texCoord0 = texCoord;
}

#elif defined(FS_BUILD)

#define EXPOSURE 2.0

uniform sampler2D screen;

out vec4 outColor;

void main() {
	const float gamma = 2.2;
	const vec3 hdrColor = texture2D(screen, texCoord0).rgb;

	//vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
	vec3 mapped = vec3(1.0) - exp(-hdrColor * EXPOSURE);

	//mapped = pow(mapped, vec3(1.0 / gamma));

	outColor = vec4(mapped, 1.0);
	//outColor = vec4(hdrColor, 1.0);
}

#endif

