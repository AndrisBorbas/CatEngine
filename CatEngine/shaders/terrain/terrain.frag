#version 450

layout( set = 0, binding = 1 ) uniform sampler2D samplerHeight;
layout( set = 0, binding = 2 ) uniform sampler2D samplerTexture;

layout( location = 0 ) in vec3 normal;
layout( location = 1 ) in vec2 uv;
layout( location = 2 ) in vec3 view;
layout( location = 3 ) in vec3 light;
layout( location = 4 ) in vec3 eyePos;
layout( location = 5 ) in vec3 worldPos;
layout( location = 6 ) in float uvScale;
layout( location = 7 ) in float height;

layout( location = 0 ) out vec4 outColor;

float fog( float density )
{
	const float LOG2 = -1.442695;
	float dist = gl_FragCoord.z / gl_FragCoord.w * 0.01;
	float d = density * dist;
	return 1.0 - clamp( exp2( d * d * LOG2 ), 0.0, 1.0 );
}

void main()
{
	vec3 N = normalize( normal );
	vec3 L = normalize( light );
	vec3 ambient = vec3( 0.5 );
	vec3 diffuse = max( dot( N, L ), 0.0 ) * vec3( 1.0 );


	vec4 color = vec4( ( ambient + diffuse ) * texture( samplerTexture, vec2( uv * uvScale ) ).rgb, 1.0 );

	const vec4 fogColor = mix( color, vec4( 0.47, 0.5, 0.67, 0.0 ), fog( 0.25 ) );
	const float mh = clamp( height * 3.0, 0.0, 1.0 );
	const vec4 heightColor = mix( vec4( 0.5, 0.5, 0.5, 1.0 ), vec4( 1.5, 1.5, 1.5, 1.0 ), mh );
	outColor = heightColor * fogColor;
}
