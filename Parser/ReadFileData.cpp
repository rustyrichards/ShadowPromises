#include "pch.h"
#include "ReadFileData.h"

void ReadFileData::dropMappedFileIfOpen(bool forceCleanupBuffer)
{
	if (forceCleanupBuffer)
	{
		if (bufferIsAllocated)
		{
			delete[] buffer;
			bufferIsAllocated = false;
		}
	}

	if (!bufferIsAllocated)
	{
		// Cannot reuse if it isn't an allocated buffer, of if the forceCleanupBuffer just deleted the buffer
		buffer = NULL;
		byteCount = 0;
		usedByteCount = 0;
	}

	if (NULL != mappedFile)
	{
		mappedFile->close();
		mappedFile = NULL;
	}
}

ReadFileData::~ReadFileData()
{
	dropMappedFileIfOpen(true);
}

const char* ReadFileData::useExistingBuffer(const char* existingBuffer, size_t elemCount)
{
	dropMappedFileIfOpen(true);

	usedByteCount = byteCount = elemCount * sizeof(char);

	return buffer = existingBuffer;
}


const char* ReadFileData::readInFile(boost::filesystem::path filePath)
{
	dropMappedFileIfOpen(true);

	mappedFile = new mapped_file_source();

	// Open the file mapping read-only.
	auto parms = basic_mapped_file_params(filePath);
	parms.flags = mapped_file_base::mapmode::readonly;

	mappedFile->open(parms);


	usedByteCount = byteCount = mappedFile->size();

	return buffer = (char*)mappedFile->data();
}

const char* ReadFileData::readInFile(istream& input)
{
	dropMappedFileIfOpen(true);

	// Compute the size in char,  use 1M if not supplied.
	long    inputSize = (long)input.gcount();
	long    bufferSize = inputSize + 1;
	if (0 >= inputSize) bufferSize = 1024 * 1024;
	bufferSize = (bufferSize + 1024) & ~1023;       // make it even 1K blocks

	if (byteCount < bufferSize * sizeof(char))
	{
		if (NULL != buffer) delete[]buffer;

		buffer = new char[bufferSize];
		bufferIsAllocated = true;
	}
	//else // we have enough space just use the current buffer.

	input.read((char*)buffer, bufferSize);
	usedByteCount = input.gcount() * sizeof(char);

	return buffer;
}

const char* ReadFileData::end()
{
	const char* bufferEnd = NULL;

	if (NULL != buffer)
	{
		bufferEnd = buffer + (usedByteCount / sizeof(char));
	}

	return bufferEnd;
}