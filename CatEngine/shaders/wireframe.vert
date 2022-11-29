#version 450

layout ( location = 0 ) in vec3 position;
layout ( location = 1 ) in vec3 color;
layout ( location = 2 ) in vec3 normal;
layout ( location = 3 ) in vec2 uv;

layout ( location = 0 ) out vec3 fragColor;

layout ( set = 0, binding = 0 ) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 inverseView;
	vec4 ambientLightColor; // w is intensity
}
ubo;

layout ( push_constant ) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 color;
}
push;

void main( )
{
	vec4 positionWorld = push.modelMatrix * vec4( position, 1.0 );
	gl_Position = ubo.projection * ubo.view * positionWorld;

	vec3 normalWorldSpace = normalize( mat3( push.normalMatrix ) * normal );

	vec3 directionToLight = vec3( 0.0, 0.0, 0.0 ) - positionWorld.xyz;
	float attenuation = 1.0 / dot( directionToLight, directionToLight ); // distance squared

	vec3 lightColor = vec3( 1.0, 1.0, 0.7 ) * 1.0 * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max( dot( normalWorldSpace, normalize( directionToLight ) ), 0.0f );

	if ( dot( push.color, vec3( 1.0, 1.0, 1.0 ) ) <= -1 )
	{
		fragColor = color;
	}
	else
	{
		fragColor = push.color;
	}
}
