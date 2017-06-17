#version 420
uniform vec4 DiffuseColor;
uniform sampler2D Texture;
uniform vec4 lightPos;
uniform float lightIntensityInLux;
uniform vec3 camPos;

// Material
uniform float roughness;
uniform float reflectance; // 0 to 1


in vec2 texCoord;
in vec4 vertColor;
in vec3 vertNormal;
in vec3 vertPos;

out vec4 colorOut;

const float PI = 3.14159265359;
#define ONE_OVER_PI	0.318309

vec3 Diffuse_Lambert( vec3 DiffuseColor )
{
	return DiffuseColor * ONE_OVER_PI;
}

float saturate(float i)
{
	return clamp(i,0.0,1.0);
}


// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float ndoth, float roughness)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float d = (ndoth * m2 - ndoth) * ndoth + 1.0;

	return m2 / max(PI * d * d, 1e-8);
}

float Vis_Smith_Schlick_GGX(float a, float NdV, float NdL)
{
       // Smith schlick-GGX.
    float k = a * a * 0.5f;
    float GV = NdV / (NdV * (1 - k) + k);
    float GL = NdL / (NdL * (1 - k) + k);

    return GV * GL;
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
vec3 Fresnel_Schlick(vec3 specularColor, vec3 h, vec3 v)
{
    return (specularColor + (1.0f - specularColor) * pow((1.0f - saturate(dot(v, h))), 5.0));
}

vec3 Specular(vec3 specularColor, vec3 h, vec3 v, float a, float NdL, float NdV, float NdH, float VdH)
{
    return ((D_GGX(NdH,a) * Vis_Smith_Schlick_GGX(a, NdV, NdL)) * Fresnel_Schlick(specularColor, v, h) ) / (4.0f * NdL * NdV + 0.0001f);
}

vec3 ComputeLight(vec3 albedoColor, vec3 specularColor, vec3 normal, float roughness, vec3 lightColor, vec3 lightDir, vec3 viewDir)
{
	vec3 H = normalize(viewDir + lightDir);
	float NoH = saturate(dot(normal, H));
	float NoV = saturate(dot(normal, viewDir));
	float NoL = saturate(dot(normal, lightDir));
	float VoH = saturate(dot(viewDir, H));
	float a = max(0.001f, roughness);
	
    vec3 cDiff = Diffuse_Lambert(albedoColor);
    vec3 cSpec = Specular(specularColor, H, viewDir, a, NoL, NoV, NoH, VoH);

    return lightColor * NoL * (cDiff * (1.0f - cSpec) + cSpec);
}

void main( void )
{
	vec4 albedo = texture(Texture, texCoord);
	
	// Gamma correction.
    albedo = pow(albedo, vec4(2.2f)) * DiffuseColor;
	
	vec3 V = normalize(-vertPos);
	vec3 L = normalize(lightPos.xyz);
	vec3 H = normalize(L + V);
	vec3 N = normalize(vertNormal);

	
	// 0.03 default specular value for dielectric.
	float metal = 0.25;
    vec3 realSpecularColor = mix(vec3(0.03f), albedo.xyz, vec3(metal));
	
	vec3 light = ComputeLight( albedo.xyz, realSpecularColor, N,  roughness, vec3(lightIntensityInLux), L, V);
	//vec3 color = tex.xyz;/* * Fd * lightIntensityInLux*/;
	//color += Fr * lightIntensityInLux;
	colorOut = vec4(light, 1.0);
};
