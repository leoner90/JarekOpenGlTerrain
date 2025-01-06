// VERTEX SHADER
#version 330

// Matrices
uniform mat4 matrixProjection; // position in space (perspective, near Plain etc.)
uniform mat4 matrixView; // camera
uniform mat4 matrixModelView; // combined two to simplify calculations (one operation instead of two!)

// Materials - can be one material instead of 3 
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess; // addition to specular light

//imput variables automaticly taken from a mesh (order Matter!!!! becouse they are ordered in a mesh)
in vec3 aVertex;
in vec3 aNormal;

//just to provide vertexes to fragment shader
in vec2 aTexCoord;
out vec2 texCoord0;

//gloabl vars
out vec4 color;
vec4 position;
vec3 normal;

//********** LIGHT DECLARATION **********

//AMBIENT LIGHT
struct AMBIENT
{
	vec3 color;
};

uniform AMBIENT lightAmbient, lightEmissive; // lightEmissive for bulb

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
	vec3 L = normalize(mat3(matrixView) * light.direction); // matrixView makes camera depending light // //cordinats 4d  direction 3d( mat3)
	float NdotL = dot(normal, L); // control light intesivity depending on rotation to light source
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0); // mix two colors // max(NdotL, 0) to not drop below 0 if away of light
	return color;
}

//POINT LIGHT
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint , lightPoint2;

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


	//ATTENUATION qadratic light drops 4 times
	float dist = length(matrixView * vec4(light.position, 1) - position);
	//float att = 1 / (att_const + att_linear * dist + att_quadratic * dist * dist);
	float att = 1 / (0.003 * dist * dist);


	return color * att;
}

void main(void)
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	//don't want 4D transformation matrix to transform normals – or any other direction vectors! so mat3
	normal = normalize(mat3(matrixModelView) * aNormal);

 	// calculate light
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient);
	color += DirectionalLight(lightDir);
	color += PointLight(lightPoint);

	// calculate texture coordinate //just to provide vertexes to fragment shader
	texCoord0 = aTexCoord;
}