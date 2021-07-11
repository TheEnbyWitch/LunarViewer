#if defined(VERTEX_SHADER)
#define v2p out
#define VSMain main
#elif defined(FRAGMENT_SHADER)
#define v2p in
#define FSMain main
#endif

#define NOPERSPECTIVE noperspective

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform vec2 screenSize;
uniform vec3 modelScale;

uniform ivec4 animData;
/* X = Current Frame
 * Y = Target Frame
 * Z = Frame Count
 * W = Interpolation Value 0-16384
 */

uniform float floorOffset;
uniform float lightOffset;
uniform int isMirrored;
uniform bool isAlphaTested;

uniform sampler2D textureModelSkin; // Indexed Palette Texture (LUNA: are these the right words?)
uniform sampler2D textureColormapLUT; // Colormap LUT
uniform sampler2D texturePaletteLUT; // Palette
uniform sampler2D textureNormalLUT; // Normal LUT to use with the normal index
uniform sampler2D textureVertexAnim; // Vertex Animation Position (XYZ) and Normal index (W)

#if defined(VERTEX_SHADER)
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
in vec3 vertexNormal;
in vec4 vertexTangent;

out float gl_ClipDistance[2];
#endif

NOPERSPECTIVE v2p vec2 fragTexCoord;
NOPERSPECTIVE v2p vec4 fragColor;
NOPERSPECTIVE v2p vec3 fragNormal;
NOPERSPECTIVE v2p vec4 fragTangent;
NOPERSPECTIVE v2p float fragLight;
NOPERSPECTIVE v2p float fog;

#if defined(FRAGMENT_SHADER)
out vec4 finalColor;        // Fragment output: color
#endif

const float SCREEN_WIDTH = 160.0;
const float SCREEN_HEIGHT = 120.0;

struct AnimatedVert
{
	vec3 Position;
	vec3 Normal;
};

float GetInterpolationValue(int val)
{
	return float(val) / 16384.0;
}

float easeOutElastic(float x)
{
	float c4 = (2 * 3.14) / 3;

	return pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
}

float easeOutBounce(float x) 
{
	float n1 = 7.5625;
	float d1 = 2.75;

	if (x < 1 / d1) {
		return n1 * x * x;
	} else if (x < 2 / d1) {
		return n1 * (x -= 1.5 / d1) * x + 0.75;
	} else if (x < 2.5 / d1) {
		return n1 * (x -= 2.25 / d1) * x + 0.9375;
	} else {
		return n1 * (x -= 2.625 / d1) * x + 0.984375;
	}
}

float easeOutCubic(float x)
{
	return 1 - pow(1 - x, 3);
}

vec3 Interpolate(vec3 a, vec3 b, float alpha)
{
	// LUNA: Boringgggg
	return mix(a,b,alpha);
	
	//return mix(a,b, easeOutCubic(alpha));
	//return mix(a,b, easeOutBounce(alpha));
	//return mix(a,b, easeOutElastic(alpha/3.0));
}

AnimatedVert GetLocalInterpolatedVertex(float U)
{
	AnimatedVert v;
	float interp = GetInterpolationValue(animData.w);

	// i dont trust those pesky floats so i add a little bit to make sure it will sample from the desired pixels
	float V1 = float(animData.x+0.1f) / float(animData.z);
	float V2 = float(animData.y+0.1f) / float(animData.z);

	vec4 a1 = texture2D(textureVertexAnim, vec2(U, V1)).rgba;
	vec4 a2 = texture2D(textureVertexAnim, vec2(U, V2)).rgba;

	v.Position = Interpolate(a1.rgb, a2.rgb, interp);
	v.Normal = normalize((Interpolate(texture2D(textureNormalLUT, vec2(a1.a, 0.0)).rgb,
				texture2D(textureNormalLUT, vec2(a2.a, 0.0)).rgb, interp)));

	return v;
}

AnimatedVert GetLocalVertex(float U)
{
	AnimatedVert v;

	float V2 = float(animData.y+0.1f) / float(animData.z);

	vec4 a2 = texture2D(textureVertexAnim, vec2(U, V2)).rgba;

	v.Position = a2.rgb;
	v.Normal = normalize(texture2D(textureNormalLUT, vec2(a2.a, 0.0)).rgb);

	return v;
}
#if defined(VERTEX_SHADER)
void VSMain(void)
{
	AnimatedVert v = 
#if defined(ANIM_INTERPOLATION)
		GetLocalInterpolatedVertex(vertexTangent.r)
#else 
		GetLocalVertex(vertexTangent.r)
#endif
	;

	vec3 finalVertPos = vertexPosition + (v.Position * 256.f * modelScale);
	// Convert from Z-up. I wish i didnt have to do this
	finalVertPos = vec3(finalVertPos.x, finalVertPos.z, -finalVertPos.y);

	vec3 normal = vec3(v.Normal.x, isMirrored > 0 ? -v.Normal.z : v.Normal.z, -v.Normal.y);

	normal = normalize(vec3(matNormal * vec4(normal, 1)));

  	gl_Position = mvp * vec4(finalVertPos, 1);

  	fragTexCoord = vertexTexCoord;
	vec3 fragPosition = vec3(matModel*vec4(finalVertPos, 1.0));

  	vec2 screenSpace = (gl_Position.xyz / gl_Position.w).xy;
  	screenSpace = floor(screenSpace * screenSize) / screenSize;
  	screenSpace *= gl_Position.w;
  	gl_Position.x = screenSpace.x;
  	gl_Position.y = screenSpace.y;

  	fragColor = vertexColor;

	fragLight = ((clamp((dot(normal, normalize(vec3(0.0,1,0))) + 1.0) / 2.0, 0.0, 1.0)) + (lightOffset)); 

	fragTangent = vertexTangent;

	fragNormal = normal;

	gl_ClipDistance[0] = dot(vec4(fragPosition, 1.0), vec4(0,1,0,-floorOffset));
	gl_ClipDistance[1] = dot(vec4(fragPosition, 1.0), vec4(0,-1,0,floorOffset));
}
#endif

#if defined(FRAGMENT_SHADER)
void FSMain(void)
{
	vec3 light = vec3(0.0);
	vec3 fragPosition = vec3(0.0);

	float PaletteIndex = texture2D(textureModelSkin, fragTexCoord).r;

	// Alpha Test
	if(isAlphaTested && PaletteIndex <= 0.0)
		discard;

#if RENDERMODE < 1 // Software-like lighting

	vec2 ColormapUV = vec2(PaletteIndex, 0.75-(fragLight / 2.0));

    finalColor = vec4(texture2D(textureColormapLUT, ColormapUV).rgb, 1.0);

#elif RENDERMODE == 1 // Fullbright, samples palette

    finalColor = vec4(texture2D(texturePaletteLUT, vec2(PaletteIndex, 0.0)).rgb, 1.0);

#elif RENDERMODE == 2 // Shows Normals

	finalColor = vec4((fragNormal.rgb + 1) / 2.0, 1.0);

#elif RENDERMODE == 3 // Shows UVs

	finalColor = vec4((fragTexCoord.rg + 1) / 2.0, 1.0, 1.0);

#elif RENDERMODE == 4 // Shows the model's indexed texture directly
	
	finalColor = vec4(PaletteIndex, PaletteIndex, PaletteIndex, 1.0);

#endif

	//finalColor = vec4((fragTangent.r), (fragTangent.r), (fragTangent.r), 1.0);

    //finalColor = PSXDither(finalColor, gl_FragCoord.xy);
}
#endif