#pragma once

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <vector>
#include <map>

#define MDLHeader GetMDLHeader<FMDLHeader>()

/*
*	MDL spec reference: http://tfc.duke.free.fr/coding/mdl-specs-en.html
*/

enum class EMDLFormat
{
	Unknown = 0,
	idPolyModel, 
	RavenPolyModel,
};

enum EMDLFlags
{
	EF_ROCKET		= (1 << 0 ),	/* leave a trail				*/
	EF_GRENADE		= (1 << 1 ),	/* leave a trail				*/
	EF_GIB			= (1 << 2 ),	/* leave a trail				*/
	EF_ROTATE		= (1 << 3 ),	/* rotate (bonus items)				*/
	EF_TRACER		= (1 << 4 ),	/* green split trail				*/
	EF_ZOMGIB		= (1 << 5 ),	/* small blood trail				*/
	EF_TRACER2		= (1 << 6 ),	/* orange split trail + rotate			*/
	EF_TRACER3		= (1 << 7 ),	/* purple trail					*/
	// After this point, quakespasm at least, doesnt preserve any flags apart from EF_HOLEY
	EF_FIREBALL		= (1 << 8 ),	
	EF_ICE			= (1 << 9 ),	/* Blue-white transparent trail, with gravity	*/
	EF_MIP_MAP		= (1 << 10),	/* This model has mip-maps			*/
	EF_SPIT			= (1 << 11),	/* Black transparent trail with negative light	*/
	EF_TRANSPARENT	= (1 << 12),	/* Transparent sprite				*/
	EF_SPELL		= (1 << 13),	/* Vertical spray of particles			*/
	EF_HOLEY		= (1 << 14),	/* Solid model with color 0			*/
	EF_SPECIAL_TRANS = (1 << 15),	/* Translucency through the particle table	*/
	EF_FACE_VIEW	= (1 << 16),	/* Poly Model always faces you			*/
	EF_VORP_MISSILE = (1 << 17),	/* leave a trail at top and bottom of model	*/
	EF_SET_STAFF	= (1 << 18),	/* slowly move up and left/right		*/
	EF_MAGICMISSILE = (1 << 19),	/* a trickle of blue/white particles with gravity	*/
	EF_BONESHARD	= (1 << 20),	/* a trickle of brown particles with gravity		*/
	EF_SCARAB		= (1 << 21),	/* white transparent particles with little gravity	*/
	EF_ACIDBALL		= (1 << 22),	/* Green drippy acid shit				*/
	EF_BLOODSHOT	= (1 << 23),	/* Blood rain shot trail				*/
};

// Only returns for single bits
inline const char* GetDescriptionForFlagQuake1(uint32_t flagValue)
{
	switch (flagValue)
	{
	case EF_ROCKET:
		return "Rocket Trail\0Leaves a rocket trail";
	case EF_GRENADE:
		return "Grenade Trail\0Leaves a grenade trail";
	case EF_GIB:
		return "Gib Trail\0Leaves a blood trail";
	case EF_ROTATE:
		return "Rotate\0Rotates the model around the up axis (Z)";
	case EF_TRACER:
		return "Tracer\0Leaves a green trail";
	case EF_ZOMGIB:
		return "Zombie Gib\0Leaves a small blood trail";
	case EF_TRACER2:
		return "Tracer 2\0Leaves an orange trail";
	case EF_TRACER3:
		return "Tracer 3\0Leaves a purple trail";
	case EF_HOLEY:
		return "Use Alpha Test (limited)\0If a texel's index is 0, it will be made transparent. Only available in some source ports!";
	default:
		return "\0\0";
	}
}

inline const char* GetDescriptionForFlagHexen2(uint32_t flagValue)
{
	switch (flagValue)
	{
	case EF_ROCKET:
		return "Rocket Trail\0Leaves a rocket trail";
	case EF_GRENADE:
		return "Grenade Trail\0Leaves a grenade trail";
	case EF_GIB:
		return "Gib Trail\0Leaves a blood trail";
	case EF_ROTATE:
		return "Rotate\0Rotates the model around the up axis (Z)";
	case EF_TRACER:
		return "Tracer\0Leaves a green trail";
	case EF_ZOMGIB:
		return "Zombie Gib\0Leaves a small blood trail";
	case EF_TRACER2:
		return "Tracer 2\0Leaves an orange trail";
	case EF_TRACER3:
		return "Tracer 3\0Leaves a purple trail";
	case EF_FIREBALL:
		return "Fireball Trail\0Leaves a transparent yellow trail in all directions";
	case EF_ICE:
		return "Ice Trail\0Leaves a transparent blue trail with gravity";
	case EF_MIP_MAP:
		return "Uses Far Mip\0TODO: Write description for this";
	case EF_SPIT:
		return "Spit Trail\0Leaves a black transparent trail";
	case EF_TRANSPARENT:
		return "Use Transparency (Sprite Model)\0Only works on sprite models";
	case EF_SPELL:
		return "Spell FX\0TODO: Write description for this";
	case EF_HOLEY:
		return "Use Alpha Test\0If a texel's index is 0, it will be made transparent";
	case EF_SPECIAL_TRANS:
		return "Use Special Transparency\0TODO: Write description for this";
	case EF_FACE_VIEW:
		return "Face View\0Model will always face the camera";
	case EF_VORP_MISSILE:
		return "Vorp Missile\0TODO: Write description for this";
	case EF_SET_STAFF:
		return "Set Staff\0Model will slowly move up and left and right";
	case EF_MAGICMISSILE:
		return "Magic Missile\0";
	case EF_BONESHARD:
		return "Bone Shard\0";
	case EF_SCARAB:
		return "Scarab\0";
	case EF_ACIDBALL:
		return "Acid Ball\0";
	case EF_BLOODSHOT:
		return "Bloodshot\0";
	default:
		return "\0\0";
	}
}

struct FMDLInfo
{
	uint32_t Magic;
	uint32_t Version;
};

struct FMDLHeader
{
	FMDLInfo Info;

	float Scale[3];
	float Translate[3];
	float BoundingRadius;
	Vector3 EyePosition;

	uint32_t NumSkins;
	uint32_t SkinWidth;
	uint32_t SkinHeight;

	uint32_t NumVerts;
	uint32_t NumTris;
	uint32_t NumFrames;

	uint32_t SyncType;
	uint32_t Flags;
	float Size;
};

struct FMDLHeader_IDPO : FMDLHeader
{

};

struct FMDLHeader_RAPO : FMDLHeader
{
	uint32_t NumTexCoords;
};

struct FMDLSkinBase
{
	// 0 = single texture
	// 1 = group of textures (its animated!)
	uint32_t Group; 
};

struct FMDLSkin : FMDLSkinBase
{
	// Textures are 8bit per pixel
	uint8_t* Data;
};

struct FMDLGroupSkin : FMDLSkinBase
{
	uint32_t NumTex;		/* number of pics */
	float* Time;	/* time duration for each pic */
	
	// Textures are 8bit per pixel
	uint8_t** Data;
};

struct FMDLTexCoord
{
	uint32_t OnSeam;
	uint32_t S;
	uint32_t T;
};

struct FMDLTriangle
{
	uint32_t IsFrontFace; // 0 = backface, 1 = frontface
	//uint32_t Vertex[3];
	union
	{
		uint32_t Vertex[3];
		struct RAPOTri_s
		{
			uint16_t Vertex[3];
			uint16_t ST[3];
		} RAPOTri;
	};
};

struct FMDLTriangle_IDPO : FMDLTriangle
{
	uint32_t Vertex[3];
};

struct FMDLTriangle_RAPO : FMDLTriangle
{
	uint16_t Vertex[3];
	uint16_t ST[3];
};

struct FMDLVertex
{
	uint8_t Position[3];
	uint8_t NormalIndex;
};

struct FMDLFrameBase
{
	int Type; // 0 = simple, not 0 = group
};

struct FMDLSimpleFrame : FMDLFrameBase
{
	FMDLVertex BBoxMin;
	FMDLVertex BBoxMax;
	char Name[16];
	FMDLVertex* Vertex;
};

struct FMDLGroupFrame : FMDLFrameBase
{
	FMDLVertex BBoxMin;
	FMDLVertex BBoxMax;
	float Time; // Interval between frames

	FMDLSimpleFrame* Frames;
};

struct FMDLShaderAnimData
{
	int CurrentFrame = 0;
	int TargetFrame = 0;
	int FrameCount = 0;
	int Interpolate = 0; // LUNA: so that i can store it in a single IVec4
	float fInterpolate = 0.0f;

	void SetInterpolate(float interp)
	{
		Interpolate = interp * 16384;
	}

	float GetInterpolate()
	{
		return (float)Interpolate / 16384.f;
	}
};

struct FMDLAnimation
{
	char Name[16] = "none";
	uint32_t Begin = (uint32_t)-1; // Purposefully make it the biggest uint32
	uint32_t End = 0;
};

class CMDL
{
public:
	CMDL(std::string Path);
	~CMDL();

	void Init(std::string Path);

	static void Setup();

	// Refreshes the model
	void RefreshModel();

	// Updates the model
	void UpdateModel();

	void CleanupModel();

	// Uses raylib to draw the model
	void DrawModel();
	void Frame(float delta, uint32_t begin, uint32_t end);

	void ChangeRenderMode(/*LunarRenderMode*/ uint32_t RenderMode, bool UseAnimInterpolation, bool Force = false);

	std::string Path;

	inline Vector3 GetTransformedVertexPosition(uint8_t vertexPos[3])
	{
		float R[3];

		for (int i = 0; i < 3; i++)
		{
			R[i] = ((MDLHeader.Scale[i] * (float)vertexPos[i]) + MDLHeader.Translate[i]);
		}

		return Vector3 { R[0], R[1], R[2] };
	}

	inline bool IsValid()
	{
		return _isValid;
	}

	inline bool HasGroupSkins()
	{
		return MDLHeader.NumSkins > 1;
	}

	inline uint32_t GetSkinByteLength()
	{
		return MDLHeader.SkinWidth * MDLHeader.SkinHeight;
	}

	inline Vector2 GetUV(const FMDLTexCoord& TC, const FMDLTriangle &Tri)
	{
		return Vector2 {
			((float)TC.S + (TC.OnSeam > 0 && Tri.IsFrontFace < 1 ? MDLHeader.SkinWidth * 0.5f : 0.0f) + 0.5f) / (float)MDLHeader.SkinWidth,
			((float)TC.T + 0.5f) / (float)MDLHeader.SkinHeight
		};
	}

	EMDLFormat MDLFormat;

	union
	{
		uint32_t MDLHeaderMagic;
		FMDLHeader_IDPO MDLHeaderIDPO;
		FMDLHeader_RAPO MDLHeaderRAPO;
	} MDLHeaderUnion;

	template<typename T>
	constexpr T& GetMDLHeader()
	{
		return *(T*)(&MDLHeaderUnion);
	}

	int CurrentFrame;

//private:
	bool _isValid;

	bool HasRaylibMesh;
	Model Model;
	Mesh Mesh;
	Material Material;
	Shader Shader = { 0 };
	Image *_images;
	Texture2D *_textures;
	LunarRenderMode CurrentRenderMode = (LunarRenderMode)-1; // force an abysmally large rendermode at first
	bool UseAnimInterpolation = true;

	FMDLShaderAnimData AnimData;

	Texture2D SimpleFrameTexture;

	uint32_t TextureCount;
	FMDLSkinBase **Skins;
	FMDLTexCoord *TexCoords;
	FMDLTriangle *Triangles;
	FMDLVertex *Vertices;

	FMDLFrameBase** Frames;

	std::vector<FMDLAnimation> AnimationSets;

};

