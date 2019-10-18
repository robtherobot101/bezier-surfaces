 #version 400
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
out vec4 theColour;
out vec2 texCoord;
out float height;
out float ripple;

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;
uniform int t;

vec4 white = vec4(1.0);
vec4 grey = vec4(0.2);

void main() {
	
	vec3 normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz,
						          gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz));
	
		
	for(int i = 0; i < gl_in.length(); i++)
	{
		vec4 position = gl_in[i].gl_Position;
		vec4 ambientColour = grey;
		vec3 lightDir = normalize(lightPos.xyz - position.xyz);
		float lambertian = max(dot(lightDir,normal), 0.0);
		float specular = 0.0;
		
		if(lambertian > 0.0) {

			vec3 viewDir = normalize(-position.xyz);
			vec3 halfDir = normalize(lightDir + viewDir);
		}
		theColour = ambientColour +
					lambertian * white;
		gl_Position = mvpMatrix * gl_in[i].gl_Position;
		texCoord = position.xz / 10;
		height = position.y;
		ripple = 0.1 * sin((position.x + sin(position.z + t / 10.0) + 0.4 * sin(t / 4.0)) * 1.5) + 0.9;
		EmitVertex();
	}
	EndPrimitive();
}
