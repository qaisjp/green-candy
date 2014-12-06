/* You can write ONE header per C BLOCk using these macros */
#define SKIP_HEADER()\
	uint32 bytesWritten = 0;\
	uint32 headerPos = rw.tellp();\
    rw.seekp(0x0C, std::ios::cur);

#define WRITE_HEADER(chunkType)\
	uint32 oldPos = rw.tellp();\
    rw.seekp(headerPos, std::ios::beg);\
	header.type = (chunkType);\
	header.length = bytesWritten;\
	bytesWritten += header.write(rw);\
	writtenBytesReturn = bytesWritten;\
    rw.seekp(oldPos, std::ios::beg);