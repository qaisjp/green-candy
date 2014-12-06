struct MatFx
{
	uint32 type;

	float32 bumpCoefficient;
	float32 envCoefficient;
	float32 srcBlend;
	float32 destBlend;

	bool hasTex1;
	Texture tex1;
	bool hasTex2;
	Texture tex2;
	bool hasDualPassMap;
	Texture dualPassMap;

	void dump(std::string ind = "");

	MatFx(void);
};

struct Material
{
	uint32 flags;
	uint8 color[4];
	uint32 unknown;
	bool hasTex;
	float32 surfaceProps[3]; /* ambient, specular, diffuse */

	Texture texture;

	/* Extensions */

	/* right to render */
	bool hasRightToRender;
	uint32 rightToRenderVal1;
	uint32 rightToRenderVal2;

	/* matfx */
	bool hasMatFx;
	MatFx *matFx;

	/* reflection mat */
	bool hasReflectionMat;
	float32 reflectionChannelAmount[4];
	float32 reflectionIntensity;

	/* specular mat */
	bool hasSpecularMat;
	float32 specularLevel;
	std::string specularName;

	/* uv anim */
	// to do

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 write(std::ostream &dff);

	void dump(uint32 index, std::string ind = "");

	Material(void);
	Material(const Material &orig);
	Material &operator=(const Material &that);
	~Material(void);
};