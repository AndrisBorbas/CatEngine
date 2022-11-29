#version 450

layout( set = 0, binding = 0 ) uniform TerrainUBO
{
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor;
	vec4 lightPos;
	vec4 frustumPlanes[6];
	vec2 viewportDimensions;
	float displacementFactor;
	float tessellationFactor;
	float tessellatedEdgeSize;
	float uvScale;
}
ubo;

layout( set = 0, binding = 1 ) uniform sampler2D samplerHeight;

layout( vertices = 4 ) out;

layout( location = 0 ) in vec3 inNormal[];
layout( location = 1 ) in vec2 inUV[];

layout( location = 0 ) out vec3 outNormal[4];
layout( location = 1 ) out vec2 outUV[4];

float screenSpaceTessFactor( vec4 p0, vec4 p1 )
{
	vec4 midPoint = 0.5 * ( p0 + p1 );
	float radius = distance( p0, p1 ) / 2.0;

	vec4 v0 = ubo.view * midPoint;

	vec4 clip0 = ( ubo.projection * ( v0 - vec4( radius, vec3( 0.0 ) ) ) );
	vec4 clip1 = ( ubo.projection * ( v0 + vec4( radius, vec3( 0.0 ) ) ) );

	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= ubo.viewportDimensions;
	clip1.xy *= ubo.viewportDimensions;

	return clamp( distance( clip0, clip1 ) / ubo.tessellatedEdgeSize * ubo.tessellationFactor, 1.0, 64.0 );
}

bool frustumCheck()
{
	const float radius = 8.0f;
	vec4 pos = gl_in[gl_InvocationID].gl_Position;
	pos.y += textureLod( samplerHeight, inUV[0], 0.0 ).r * ubo.displacementFactor;

	for ( int i = 0; i < 6; i++ )
	{
		if ( dot( pos, ubo.frustumPlanes[i] ) + radius < 0.0 )
		{
			return false;
		}
	}
	return true;
}

void main()
{
	if ( gl_InvocationID == 0 )
	{
		if ( !frustumCheck() )
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
		}
		else
		{
			if ( ubo.tessellationFactor > 0.0 )
			{
				gl_TessLevelOuter[0] = screenSpaceTessFactor( gl_in[3].gl_Position, gl_in[0].gl_Position );
				gl_TessLevelOuter[1] = screenSpaceTessFactor( gl_in[0].gl_Position, gl_in[1].gl_Position );
				gl_TessLevelOuter[2] = screenSpaceTessFactor( gl_in[1].gl_Position, gl_in[2].gl_Position );
				gl_TessLevelOuter[3] = screenSpaceTessFactor( gl_in[2].gl_Position, gl_in[3].gl_Position );
				gl_TessLevelInner[0] = mix( gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5 );
				gl_TessLevelInner[1] = mix( gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5 );
			}
			else
			{
				gl_TessLevelInner[0] = 1.0;
				gl_TessLevelInner[1] = 1.0;
				gl_TessLevelOuter[0] = 1.0;
				gl_TessLevelOuter[1] = 1.0;
				gl_TessLevelOuter[2] = 1.0;
				gl_TessLevelOuter[3] = 1.0;
			}
		}
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	outNormal[gl_InvocationID] = inNormal[gl_InvocationID];
	outUV[gl_InvocationID] = inUV[gl_InvocationID];
}
