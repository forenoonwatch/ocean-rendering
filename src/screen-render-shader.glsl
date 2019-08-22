#include "common.glh"

varying vec2 texCoord0;

#if defined(VS_BUILD)

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
	texCoord0 = texCoord;
}

#elif defined(FS_BUILD)

uniform sampler2D screen;

out vec4 outColor;

void main() {
	outColor = texture2D(screen, texCoord0);
}

#endif
