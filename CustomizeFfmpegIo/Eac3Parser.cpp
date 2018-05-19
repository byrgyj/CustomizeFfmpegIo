#include "StdAfx.h"
#include "Eac3Parser.h"


Eac3Parser::Eac3Parser(std::string path) : mPath(path), mFile(NULL) {
	mLogFile = fopen("eac_info.log", "w");
}

Eac3Parser::~Eac3Parser(){
	if (mFile != NULL){
		fclose(mFile);
		mFile = NULL;
	}

	if (mLogFile != NULL){
		fclose(mLogFile);
		mLogFile = NULL;
	}
}

bool Eac3Parser::init(){
	mFile = fopen(mPath.c_str(), "rb");
	if (mFile == NULL){
		return false;
	}

	fseek(mFile, 820, SEEK_SET);


	return true;
}

void Eac3Parser::parse(){
	int bufferSize = 1024 * 1024 * 2;
	char * buffer = new char[bufferSize];

	int currentOffset = 820;
	int leftSize = 0;
	int frameCount = 0;

	while(!feof(mFile)){
		int readSize = fread(buffer + leftSize, 1, bufferSize - leftSize, mFile);


		readSize += leftSize;
		if (readSize <= 0){
			break;
		}


		int index = 0;
		char *pData = buffer;
		int pos = findSyncByte(pData, readSize);
		if (pos < 0){
			break;
		}

		index = pos;
		while(index < readSize){
			const char *pdata = buffer + index;

			if (index  +  4  > readSize){ // 剩余数据小于4字节
				memcpy(buffer, buffer + index, readSize - index);
				leftSize = readSize - index;

				currentOffset += index;
				break;
			}

			int frameSize = currentFrameSize(pdata);
			if (frameSize != -1){
				if (index + frameSize > readSize){
					memcpy(buffer, buffer + index, readSize - index);
					leftSize = readSize - index;
					currentOffset += index;
					break;
				} else {
					
					//printf("index: %d, offset:%0x, frmsize:%d \n", frameCount++, currentOffset + index, frameSize);
					fprintf(mLogFile, "index: %d, offset:%0x, frmsize:%d \n", frameCount++, currentOffset + index, frameSize);
					index += frameSize;

				}
			} else {
				index++;
			}
		}
	}

	if (buffer != NULL){
		delete []buffer;
	}
}

int Eac3Parser::findSyncByte(const char *buffer, int bufferLength){
	if (buffer == 0){
		return -1;
	}

	int index = 0; 
	while (index < bufferLength){
		if (*(buffer + index) == 0x0B){
			if (index + 1 < bufferLength && *(buffer + index + 1) == 0x77){
				return index;
			}
		}

		index++;
	}

	return -1;
}
int Eac3Parser::currentFrameSize(const char *frame){
	if (frame == NULL){
		return -1;
	}

	if (*frame != 0x0B && *(frame+1) != 0xFF){
		return -1;
	}

	unsigned char low = frame[3];
	unsigned int frameSize = (frame[2] & 0x07) << 8 | low;

	unsigned result = (frameSize + 1) * 2;
	return result;
}