struct Atomic
{
	int32 frameIndex;
	int32 geometryIndex;

	/* Extensions */

	/* right to render */
	bool hasRightToRender;
	uint32 rightToRenderVal1;
	uint32 rightToRenderVal2;

	/* particles */
	bool hasParticles;
	uint32 particlesVal;

	/* pipelineset */
	bool hasPipelineSet;
	uint32 pipelineSetVal;

	/* material fx */
	bool hasMaterialFx;
	uint32 materialFxVal;

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 write(std::ostream &dff);
	void dump(uint32 index, std::string ind = "");

	Atomic(void);
};

struct MeshExtension
{
	uint32 unknown;

	std::vector<float32> vertices;
	std::vector<float32> texCoords;
	std::vector<uint8> vertexColors;
	std::vector<uint16> faces;
	std::vector<uint16> assignment;

	std::vector<std::string> textureName;
	std::vector<std::string> maskName;
	std::vector<float32> unknowns;
};

struct Split
{
	uint32 matIndex;
	std::vector<uint32> indices;	
};

struct Geometry
{
	uint32 flags;
	uint32 numUVs;
	bool hasNativeGeometry;

	uint32 vertexCount;
	std::vector<uint16> faces;
	std::vector<uint8> vertexColors;
	std::vector<float32> texCoords[8];

	/* morph target (only one) */
	float32 boundingSphere[4];
	uint32 hasPositions;
	uint32 hasNormals;
	std::vector<float32> vertices;
	std::vector<float32> normals;

	std::vector<Material> materialList;

	/* Extensions */

	/* bin mesh */
	uint32 faceType;
	uint32 numIndices;
	std::vector<Split> splits;

	/* skin */
	bool hasSkin;
	uint32 boneCount;
	uint32 specialIndexCount;
	uint32 unknown1;
	uint32 unknown2;
	std::vector<uint8> specialIndices;
	std::vector<uint32> vertexBoneIndices;
	std::vector<float32> vertexBoneWeights;
	std::vector<float32> inverseMatrices;

	/* mesh extension */
	bool hasMeshExtension;
	MeshExtension *meshExtension;

	/* night vertex colors */
	bool hasNightColors;
	uint32 nightColorsUnknown;
	std::vector<uint8> nightColors;

	/* 2dfx */
	// to do

	/* morph (only flag) */
	bool hasMorph;

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	void readMeshExtension(std::istream &dff);
	uint32 write(std::ostream &dff);
	uint32 writeMeshExtension(std::ostream &dff);

	void cleanUp(void);

	void dump(uint32 index, std::string ind = "", bool detailed = false);

	Geometry(void);
	Geometry(const Geometry &orig);
	Geometry &operator= (const Geometry &other);
	~Geometry(void);
private:
	void readPs2NativeData(std::istream &dff);
	void readXboxNativeData(std::istream &dff);
	void readXboxNativeSkin(std::istream &dff);
	void readOglNativeData(std::istream &dff, int size);
	void readNativeSkinMatrices(std::istream &dff);
	bool isDegenerateFace(uint32 i, uint32 j, uint32 k);
	void generateFaces(void);
	void deleteOverlapping(std::vector<uint32> &typesRead, uint32 split);
	void readData(uint32 vertexCount, uint32 type, // native data block
                      uint32 split, std::istream &dff);

	uint32 addTempVertexIfNew(uint32 index);
};

struct Clump
{
	std::vector<Atomic> atomicList;
	std::vector<Frame> frameList;
	std::vector<Geometry> geometryList;

	/* Extensions */
	/* collision file */
	// to do

	/* functions */
	void read(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 write(std::ostream &dff);
	void dump(bool detailed = false);
	void clear(void);
};