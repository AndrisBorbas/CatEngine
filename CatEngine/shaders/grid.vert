// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

#version 450

// layout ( location = 0 ) in vec3 position;

layout ( location = 0 ) out float near; //0.01
layout ( location = 1 ) out float far; //100
layout ( location = 2 ) out vec3 nearPoint;
layout ( location = 3 ) out vec3 farPoint;
layout ( location = 4 ) out mat4 fragView;
layout ( location = 8 ) out mat4 fragProjection;


layout ( set = 0, binding = 0 ) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 inverseView;
	vec4 ambientLightColor; // w = intensity
} ubo;

layout ( push_constant ) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
}
push;

// Grid position are in xy clipped space
vec4 gridPlane[6] = vec4[](
	vec4( 1, 1, 0, 1 ), vec4( -1, -1, 0, 1 ), vec4( -1, 1, 0, 1 ),
	vec4( -1, -1, 0, 1 ), vec4( 1, 1, 0, 1 ), vec4( 1, -1, 0, 1 )
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection ) {
	mat4 viewInv = inverse( view );
	mat4 projInv = inverse( projection );
	vec4 unprojectedPoint = viewInv * projInv * vec4( x, y, z, 1.0 );
	return unprojectedPoint.xyz / unprojectedPoint.w;
}

// normal vertice projection
void main( ) {
	vec4 p = gridPlane[gl_VertexIndex];
	near = 0.1;
	far = 100.0;
	//TODO: change to send position instead of matrix
	nearPoint = UnprojectPoint( ( push.modelMatrix * p ).x, ( push.modelMatrix * p ).y, 0.0, ubo.view, ubo.projection ).xyz; // unprojecting on the near plane
	farPoint = UnprojectPoint( ( push.modelMatrix * p ).x, ( push.modelMatrix * p ).y, 1.0, ubo.view, ubo.projection ).xyz; // unprojecting on the far plane
	gl_Position = /* push.modelMatrix * */ vec4( p.xyz, 1.0 ); // using directly the clipped coordinates
	fragView = ubo.view;
	fragProjection = ubo.projection;
}
