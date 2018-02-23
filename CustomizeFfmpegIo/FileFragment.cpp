#include "StdAfx.h"
#include "FileFragment.h"

struct DataSegment{
	int offset;
	int dataSize;
};

DataSegment segments[] = {
	{1227, 52 + 52 + 1900 + 2132941 },
	{2136172, 52 + 52 + 1344 + 1168020 },
	{3305640, 52 + 52 + 1340 + 1058606 },
	{4365690, 52 + 52 + 1344 + 1854550 },
	{6221688, 52 + 52 + 1344 + 1448918 },
	{7672054, 52 + 52 + 1340 + 1559102 },
	{9232600, 52 + 52 + 972 + 1235772 + 338}
};

FileFragment::FileFragment(std::string file){
	mFile = fopen(file.c_str(), "rb");
}


FileFragment::~FileFragment(void){
	if (mFile != NULL){
		fclose(mFile);
	}
}

void FileFragment::segmentFile(){
	if (mFile == NULL){
		return;
	}

	int headerSize = 24 + 1203;

	char *headerBuffer = new char[headerSize];
	memset(headerBuffer, 0, headerSize);
	fread(headerBuffer, 1, headerSize, mFile);
	saveToFile("fmp4/header.mp4", headerBuffer, headerSize);


	for (int i = 0; i < _countof(segments); i++){
		DataSegment ds = segments[i];

		char *DataBuffer = new char[ds.dataSize];
		memset(DataBuffer, 0, ds.dataSize);
		fread(DataBuffer, 1, ds.dataSize, mFile);

		char szName[64] = { 0 };
		sprintf(szName, "fmp4/data%d.m4s", i);

		saveToFile(szName, DataBuffer, ds.dataSize);
		delete []DataBuffer;
	}
}

void FileFragment::mergeFile(std::string src1, std::string src2, std::string destFile){
	char *bufferSrc1 = NULL;
	char *bufferSrc2 = NULL;

	int sizeSrc1 = getFileContent(src1, &bufferSrc1);
	int sizeSrc2 = getFileContent(src2, &bufferSrc2);

	FILE *outFile = fopen(destFile.c_str(), "wb");
	if (outFile == NULL){
		goto end;
	}

	if (sizeSrc1 > 0){
		fwrite(bufferSrc1, 1, sizeSrc1, outFile);
	}
	
	if (sizeSrc2 > 0){
		fwrite(bufferSrc2, 1, sizeSrc2, outFile);
	}
	
	fclose(outFile);

end:
	if (bufferSrc1 != NULL){
		delete []bufferSrc1;
	}
	if (bufferSrc2 != NULL){
		delete []bufferSrc2;
	}

}

int FileFragment::getFileContent(std::string file, char **buffer){
	if (file.empty() || buffer == NULL){
		return -1;
	}

	FILE *fileSrc = fopen(file.c_str(), "rb");
	if (fileSrc == NULL){
		return -1;
	}

	fseek(fileSrc, 0, SEEK_END);
	int sz = ftell(fileSrc);
	fseek(fileSrc, 0, SEEK_SET);
	*buffer = new char[sz];

	fread(*buffer, 1, sz, fileSrc);
	fclose(fileSrc);

	return sz;
}

void FileFragment::saveToFile(std::string file, char *data, int dataSize){
	FILE *outFile = fopen(file.c_str(), "wb");
	if (outFile != NULL){
		fwrite(data, 1, dataSize, outFile);
		fclose(outFile);
	}
}