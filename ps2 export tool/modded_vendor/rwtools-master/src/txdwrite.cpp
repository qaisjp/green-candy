#include <cstring>
#include <assert.h>
#include <math.h>

#include <StdInc.h>

using namespace std;

#include "streamutil.hxx"

namespace rw {

uint32 TextureDictionary::write(ostream &rw)
{
	HeaderInfo header;
    header.version = rw::rwInterface.GetVersion();
	uint32 writtenBytesReturn;

	/*
	 * writtenBytesReturn will always contain the number of bytes
	 * written in a sub-block, it is used like a return value.
	 */

	// Texture Dictionary
	SKIP_HEADER();

	// Struct
	{
		SKIP_HEADER();
		bytesWritten += writeUInt16(texList.size(), rw);
		bytesWritten += writeUInt16(this->dataStatus, rw);
        WRITE_HEADER(CHUNK_STRUCT);
	}
	bytesWritten += writtenBytesReturn;

	// Texture Natives
	for (uint32 i = 0; i < texList.size(); i++)
    {
        NativeTexture& tex = texList[i];
	    if (tex.platform == PLATFORM_D3D8 ||
	        tex.platform == PLATFORM_D3D9) {
		    bytesWritten += tex.writeD3d(rw);
        } else if (tex.platform == PLATFORM_PS2) {
            bytesWritten += tex.writePs2(rw);
	    } else {
		    cerr << "can't write platform " <<
			    texList[i].platform << endl;
	    }
	}

	// Extension
	{
		SKIP_HEADER();
		WRITE_HEADER(CHUNK_EXTENSION);
	}
	bytesWritten += writtenBytesReturn;

	WRITE_HEADER(CHUNK_TEXDICTIONARY);

	return writtenBytesReturn;
}

}
