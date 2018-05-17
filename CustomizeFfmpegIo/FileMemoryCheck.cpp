#include "StdAfx.h"
#include "FileMemoryCheck.h"


FileMemoryCheck::FileMemoryCheck(char *filePath) : mFile(NULL){
	mFile = fopen(filePath, "rb");
}


FileMemoryCheck::~FileMemoryCheck(){
	if(mFile != NULL){
		fclose(mFile);
		mFile = NULL;
	}
}

void FileMemoryCheck::process(){
	int offset = 0x420 + 13;

	int totalSize = 100 * 1024;
	char *buffer = new char[100 * 1024];
	if (mFile != NULL){
		totalSize = fread(buffer, 1, totalSize, mFile);
	}

	char *p = buffer;
	p = buffer + offset;
	p = buffer + offset + 5;

	*p = 0x83;

	FILE *newFile = fopen("newfile.mp4", "wb");
	if (newFile != NULL){
		fwrite(buffer, 1, totalSize, newFile);
		fclose(newFile);
	}

}