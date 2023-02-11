#ifndef READFILEDATA_H_INCLUDED
#define READFILEDATA_H_INCLUDED

#include "pch.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include <iosfwd>
#include <iostream>


using namespace std;
using namespace boost::iostreams;

class EXPORT ReadFileData
{
protected:
	mapped_file_source* mappedFile;
	bool bufferIsAllocated;
	const char* buffer;
	size_t	usedByteCount;
	size_t	byteCount;

	void dropMappedFileIfOpen(bool forceCleanupBuffer = false);

public:
	ReadFileData() :
		mappedFile(NULL),
		bufferIsAllocated(false),
		buffer(NULL),
		usedByteCount(),
		byteCount(0)
	{
	}

	~ReadFileData();

	const char* useExistingBuffer(const char* existingBuffer, size_t elemCount);

	const char* readInFile(boost::filesystem::path filePath);

	const char* readInFile(istream& input);

	const char* end();
};

#endif // READFILEDATA_H_INCLUDED
