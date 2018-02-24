#pragma once
#include "stdafx.h"

#define USE_VIDEO_1
class FileFragment
{
public:
	FileFragment(std::string &file);
	~FileFragment();

	void segmentFile();

	void mergeFile(std::string src1, std::string src2, std::string destFile);
private:

	int getFileContent(std::string file, char **buffer);
	void saveToFile(std::string file, char *data, int dataSize);
private:
	FILE* mFile;
};

