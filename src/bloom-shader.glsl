
#if defined(VS_BUILD)

out vec2 texCoord0;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);
	texCoord0 = texCoord;
}

#elif defined(FS_BUILD)

in vec2 texCoord0;

uniform sampler2D scene;
uniform sampler2D brightBlur;

out vec4 outColor;

void main() {
	const vec3 sceneColor = texture2D(scene, texCoord0).rgb;
	const vec3 brightColor = texture2D(brightBlur, texCoord0).rgb;

	outColor = vec4(sceneColor + brightColor, 1.0);
}

#endif
