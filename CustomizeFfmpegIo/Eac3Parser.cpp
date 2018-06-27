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

	fseek(mFile, 0x9BE, SEEK_SET);


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


///////////////////
DataBuffer::DataBuffer(std::string filePath) : mReadIndex(0), mWriteIndex(0) {
    mFile = fopen(filePath.c_str(), "rb");
    fseek(mFile, 0x9BE, SEEK_SET);
}
DataBuffer::~DataBuffer() {
    if (mFile != NULL) {
        fclose(mFile);
    }
}

char *DataBuffer::getData(int &dataSize) {
    if (mFile == NULL) {
        dataSize = 0;
        return NULL;
    }

    char *data = NULL;
    if (!feof(mFile)){
        data = new char[dataSize];
        dataSize = fread(data, 1, dataSize, mFile);
    } else {
        dataSize = 0;
    }

    return data;

//     char *data = NULL;
// 
//     if (mWriteIndex == 0) {
//         if (!feof(mFile)) {
//             fread(mBuffer, 1, mTotalBufferSize, mFile);
//             mWriteIndex = mTotalBufferSize;
//         }
//     }
// 
//     if (mWriteIndex != 0) 
//         int RequestSize = ( dataSize <= mWriteIndex   - mReadIndex ?  dataSize : mWriteIndex   - mReadIndex);
//         if (RequestSize > 0){
//             data = mBuffer + mReadIndex;
//             mReadIndex += RequestSize;
//             dataSize = RequestSize;
//         } else {
// 
//         }
//     }
// 
//     return NULL;
}

Eac3Context::Eac3Context() : mDataIndex(0) {
    mDataSource = new DataBuffer("./out_1.mp4");
    mSeekTime = 0;
}
Eac3Context::~Eac3Context() {
    if (mDataSource != NULL) {
        delete mDataSource;
    }
}

AVPacket *Eac3Context::readPacket() {
    int dataSize = 1024 * 2;

    int requestSize = dataSize;
    int dataIndex = 0;
    do 
    {
        char * data = mDataSource->getData(dataSize);
        if (data == NULL || dataSize <= 0){
            break;
        }
        memcpy(mBuffer + dataIndex, data, dataSize);
        dataIndex += dataSize;

    } while (dataIndex < requestSize);

    int pos = findSyncByte(mBuffer, dataSize);
    if (pos >= 0) {
        int size = currentFrameSize(mBuffer + pos);
        if (size > 0) {
            AVPacket *pkt = av_packet_alloc();
            av_new_packet(pkt, size);
            memcpy(pkt->data, mBuffer + pos, size);
            pkt->pts = pkt->dts = mSeekTime;
            pkt->duration = 0;
            pkt->size = size;

            mSeekTime += 1536;
            
            return pkt;
        } else {

        }
    }


    return NULL;
}


int Eac3Context::findSyncByte(const char *buffer, int bufferLength){
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

int Eac3Context::currentFrameSize(const char *frame){
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