// FRAGMENT SHADER

#version 330

in vec4 color;
out vec4 outColor;

//textures
uniform sampler2D texture0;
in vec2 texCoord0;
 



//Per Fragment light every pixel calculates the light NOMRAL MAPS WORKS ONLY PER FRAGMENT LIGHT


vec4 position;
vec3 normal;


// Matrices
uniform mat4 matrixView;


// Materials - can be one material instead of 3 
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess; // addition to specular light


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
	return color;
}


void main(void) 
{
  outColor = color;
  //outColor += PointLight(lightPoint); // per fragment light
  outColor *= texture(texture0, texCoord0);
}
