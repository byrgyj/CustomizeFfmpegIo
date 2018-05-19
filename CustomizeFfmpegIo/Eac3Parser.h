#pragma once
class Eac3Parser
{
public:
	Eac3Parser(std::string path);
	~Eac3Parser();

	bool init();

	void parse();

	int findSyncByte(const char *buffer, int bufferLength);
	int currentFrameSize(const char *frame);
private:
	FILE *mFile;
	FILE *mLogFile;
	std::string mPath;
};

