#include <cstring>
#include <assert.h>
#include <math.h>

#include <StdInc.h>

using namespace std;

#include "streamutil.hxx"

namespace rw {

uint32 TextureDictionary::write(ostream &rw)
{
    LibraryVersion version = rw::rwInterface.GetVersion();

	HeaderInfo header;
    header.setVersion( version );
	uint32 writtenBytesReturn;

	/*
	 * writtenBytesReturn will always contain the number of bytes
	 * written in a sub-block, it is used like a return value.
	 */

	// Texture Dictionary
	SKIP_HEADER();

	// Struct
    uint32 numTextures = texList.size();

    if ( numTextures > 0xFFFF )
    {
        throw RwException( "texture dictionary has too many textures" );
    }

	{
		SKIP_HEADER();

        // Write depending on version.
        if (version.rwLibMinor <= 5)
        {
            writeUInt32(numTextures, rw);
        }
        else
        {
            // Determine the recommended platform to give this TXD.
            // If we dont have any, we can write 0.
            uint16 recommendedPlatform = 0;

            if (this->hasRecommendedPlatform)
            {
                // A recommended platform can only be given if all textures are of the same platform.
                // Otherwise it is misleading.
                bool hasTexPlatform = false;
                bool hasValidPlatform = true;
                uint32 curPlatform;

                for ( uint32 n = 0; n < numTextures; n++ )
                {
                    const NativeTexture& tex = this->texList[ n ];

                    if ( !hasTexPlatform )
                    {
                        curPlatform = tex.platform;

                        hasTexPlatform = true;
                    }
                    else
                    {
                        if ( curPlatform != tex.platform )
                        {
                            hasValidPlatform = false;
                            break;
                        }
                    }
                }

                // Set it.
                if ( hasTexPlatform && hasValidPlatform )
                {
                    // TODO.
                    if ( curPlatform == PLATFORM_D3D8 )
                    {
                        recommendedPlatform = 1;
                    }
                    else if ( curPlatform == PLATFORM_D3D9 )
                    {
                        recommendedPlatform = 2;
                    }
                    else if ( curPlatform == PLATFORM_PS2 )
                    {
                        recommendedPlatform = 6;
                    }
                    else if ( curPlatform == PLATFORM_XBOX )
                    {
                        recommendedPlatform = 8;
                    }
                }
            }

		    bytesWritten += writeUInt16(numTextures, rw);
		    bytesWritten += writeUInt16(recommendedPlatform, rw);
        }
        WRITE_HEADER(CHUNK_STRUCT);
	}
	bytesWritten += writtenBytesReturn;

	// Texture Natives
	for (uint32 i = 0; i < numTextures; i++)
    {
        NativeTexture& tex = texList[i];
	    if (tex.platform == PLATFORM_D3D8 ||
	        tex.platform == PLATFORM_D3D9)
        {
		    bytesWritten += tex.writeD3d(rw);
        }
        else if (tex.platform == PLATFORM_PS2)
        {
            bytesWritten += tex.writePs2(rw);
	    }
        else if (tex.platform == PLATFORM_XBOX)
        {
            bytesWritten += tex.writeXbox(rw);
        }
        else
        {
            throw RwException( "can't write platform" );
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
