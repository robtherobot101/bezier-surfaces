 #version 400
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
in vec4 colour[];
out vec4 theColour;

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;

vec4 white = vec4(1.0);
vec4 grey = vec4(0.2);
vec4 cyan = vec4(0.0, 1.0, 1.0, 1.0);
float shininess = 100.0;

void main() {
	
	vec3 normal = normalize(cross(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz,
						          gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz));
	
		
	for(int i = 0; i < gl_in.length(); i++)
	{		
		vec4 ambientColour = colour[i] * grey;
		vec4 position = gl_in[i].gl_Position;
		vec3 lightDir = normalize(lightPos.xyz - position.xyz);
		float lambertian = max(dot(lightDir,normal), 0.0);
		float specular = 0.0;
		
		if(lambertian > 0.0) {

			vec3 viewDir = normalize(-position.xyz);
			vec3 halfDir = normalize(lightDir + viewDir);
			float specAngle = max(dot(halfDir, normal), 0.0);
			specular = pow(specAngle, shininess);
		}
		theColour = ambientColour +
					colour[i] * lambertian * white +
					white * specular;
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
