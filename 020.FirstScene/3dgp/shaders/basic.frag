// FRAGMENT SHADER

#version 330

in vec4 color;
out vec4 outColor;

uniform sampler2D texture0;
uniform sampler2D textureNormal;	//for normals
in vec2 texCoord0;

uniform bool useNormalMap;

in vec4 position;
in vec3 normal;
vec3 normalNew;


uniform float att_quadratic;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
// View Matrix
uniform mat4 matrixView;

in mat3 matrixTangent;

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

struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint0, lightPoint1;
//uniform POINT lightPoint;

struct SPOT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff;
	float attenuation;
	mat4 matrix;
};
uniform SPOT lightSpot;

vec4 PointLight(POINT light)
{
// Calculate Point Light
	vec4 color = vec4(0, 0, 0, 0);

	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	//vec3 L = normalize((vec4(light.position, 1) * matrixView) - position).xyz;

	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (att_quadratic * dist * dist);

	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	color += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1);
	return color * att;
}

vec4 SpotLight(SPOT light)
{
// HERE GOES THE CODE COPIED FROM THE POINT LIGHT FUNCTION
	vec4 color = vec4(0, 0, 0, 0);

	vec3 L = normalize(light.matrix * vec4(light.position, 1) - position).xyz;

	float dist = length(light.matrix * vec4(light.position, 1) - position);
	light.attenuation = 1 / (att_quadratic * dist * dist);

	float NdotL = dot(normal, L);
	//color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	color += vec4(materialSpecular * light.specular, 1);
	//color += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1);
	//return color * attenuation;
// HERE GOES THE NEW CODE TO DETERMINE THE SPOT FACTOR
	
	vec3 direction = normalize(mat3(light.matrix) * light.direction);
	float spotFactor = dot(-L, direction);
	spotFactor = acos(spotFactor);

	if (spotFactor <= light.cutoff)
	{
		spotFactor = pow(spotFactor, lightSpot.attenuation);
	}

	else
	{
		spotFactor = 0;
	}

// assuming that the Point Light value is stored as color and we have calculated spotFactor:
	return spotFactor * color;
}

void main(void) 
{
	if (useNormalMap)
	{
		normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
		normalNew = normalize(matrixTangent * normalNew);
	}
	else
	{
		normalNew = normal;
	}

  outColor = color;
  outColor += DirectionalLight(lightDir);
  outColor += PointLight(lightPoint0);
  outColor += PointLight(lightPoint1);
  outColor += SpotLight(lightSpot);
  outColor *= texture(texture0, texCoord0);
}
//