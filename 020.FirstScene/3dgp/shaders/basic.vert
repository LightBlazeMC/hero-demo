// VERTEX SHADER
#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

uniform mat4 matrixShadow;
out vec4 shadowCoord;

uniform float fogDensity;

in vec3 aVertex;
in vec3 aNormal;

out float fogFactor;

in vec2 aTexCoord;
out vec2 texCoord0;

out vec4 color;
out vec4 position;
out vec3 normal;

in vec3 aTangent;
in vec3 aBiTangent;
out mat3 matrixTangent;

in ivec4 aBoneId; // Bone Ids
in vec4 aBoneWeight; // Bone Weights

// Bone Transforms
#define MAX_BONES 250
uniform mat4 bones[MAX_BONES];

// Light declarations
struct AMBIENT
{
	vec3 color;
};
uniform AMBIENT lightAmbient;

vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

void main(void)
{
	mat4 matrixBone;

	if (aBoneWeight[0] == 0.0)
    {
        matrixBone = mat4(1);
    }
    else
    {
		matrixBone = (bones[aBoneId[0]] * aBoneWeight[0] +
							bones[aBoneId[1]] * aBoneWeight[1] +
							bones[aBoneId[2]] * aBoneWeight[2] +
							bones[aBoneId[3]] * aBoneWeight[3]);
    }
	

	// calculate position
	position = matrixModelView * matrixBone * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate tangent local system transformation
	vec3 tangent = normalize(mat3(matrixModelView) * aTangent);
	vec3 biTangent = normalize(mat3(matrixModelView) * aBiTangent);
	matrixTangent = mat3(tangent, biTangent, normal);

	// calculate light
	color = vec4(0, 0, 0, 1);

	normal = normalize(mat3(matrixModelView) * mat3(matrixBone) * aNormal);

	color += AmbientLight(lightAmbient);
	//color += PointLight(lightPoint1);
	//color += PointLight(lightPoint2);

	// calculate texture coordinate
	texCoord0 = aTexCoord;

	// calculate shadow coordinate – using the Shadow Matrix
	mat4 matrixModel = inverse(matrixView) * matrixModelView;
	shadowCoord = matrixShadow * matrixModel * vec4(aVertex + aNormal * 0.1, 1);

	fogFactor = exp2(-fogDensity * length(position));
}