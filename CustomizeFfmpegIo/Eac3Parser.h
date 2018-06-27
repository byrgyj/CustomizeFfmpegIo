#pragma once
extern "C"{
#include <libavformat/avformat.h>
};
class Eac3Parser
{
public:
	Eac3Parser(std::string path);
	~Eac3Parser();

	bool init();

	void parse();

	int findSyncByte(const char *buffer, int bufferLength);
	int currentFrameSize(const char *frame);

    ///
    int getData(char *dataBuffer, int dataSize);
    

private:
	FILE *mFile;
	FILE *mLogFile;
	std::string mPath;
};

class DataBuffer {
public:
    DataBuffer(std::string filePath);
    ~DataBuffer();

    char *getData(int &dataSize);
private:
    //const int mTotalBufferSize = 1024 * 6;
    char mBuffer[ 1024 * 6];

    int mReadIndex;
    int mWriteIndex;
    FILE *mFile;
};


class Eac3Context {
public:
    Eac3Context();
    ~Eac3Context();

    AVPacket *readPacket();
    int findSyncByte(const char *buffer, int bufferLength);
    int currentFrameSize(const char *frame);
private:
    DataBuffer *mDataSource;

    char mBuffer[1024 * 4];
    int mDataIndex;
    int mSeekTime;
};

