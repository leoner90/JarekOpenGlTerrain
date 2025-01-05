// VERTEX SHADER
#version 330

//AMBIENT LIGHT
struct AMBIENT
{
	vec3 color;
};

uniform AMBIENT lightAmbient;

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

in vec3 aVertex;
in vec3 aNormal;

//just to provide vertexes to fragment shader
in vec2 aTexCoord;
out vec2 texCoord0;

out vec4 color;
vec4 position;
vec3 normal;


vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

//**** Directional light
struct DIRECTIONAL
{
	vec3 direction;
	vec3 diffuse;
};

uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	return color;
}

//POINT LIGHT
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint;

vec4 PointLight(POINT light)
{
	// Calculate point Light
	vec4 color = vec4(0, 0, 0, 0);

	vec3 L = vec3(normalize(matrixView * vec4(light.position,1) - position));
	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);

	//shine
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	color += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1);
	return color;
}

void main(void)
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);

	gl_Position = matrixProjection * position;

	// calculate light
	color = vec4(0, 0, 0, 1);

	//don't want 4D transformation matrix to transform normals – or any other direction vectors! so mat3
	normal = normalize(mat3(matrixModelView) * aNormal);

 
	color += AmbientLight(lightAmbient);
	color += DirectionalLight(lightDir);
	color += PointLight(lightPoint);

	// calculate texture coordinate //just to provide vertexes to fragment shader
	texCoord0 = aTexCoord;

}