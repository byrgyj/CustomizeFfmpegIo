#pragma once
class FileMemoryCheck
{
public:
	FileMemoryCheck(char *filePath);
	~FileMemoryCheck(void);

	void process();

private:
	FILE *mFile;
};

