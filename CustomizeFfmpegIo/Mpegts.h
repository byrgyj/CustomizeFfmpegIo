#pragma once
#define TS_PACKET_SIZE 188
#include <stdint.h>
#include <vector>
#include <map>

typedef struct TsPatProgram
{
	unsigned program_number    :16; //节目号
	unsigned program_map_PID   :13;   //节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
}TsPatProgram;

//PAT表结构体
typedef struct TsPatTable
{
	uint8_t tableId                        : 8; //固定为0x00 ，标志是该表是PAT
	uint8_t sectionSyntaxIndicator        : 1; //段语法标志位，固定为1
	uint8_t zero                            : 1; //0
	uint8_t reserved_1                        : 2; // 保留位
	unsigned sectionLength                    : 12; //表示这个字节后面有用的字节数，包括CRC32
	unsigned transportStreamId            : 16; //该传输流的ID，区别于一个网络中其它多路复用的流
	uint8_t reserved_2                        : 2;// 保留位
	uint8_t versionNumber                    : 5; //范围0-31，表示PAT的版本号
	uint8_t currentNextIndicator            : 1; //发送的PAT是当前有效还是下一个PAT有效
	uint8_t sectionNumber                    : 8; //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
	uint8_t lastSectionNumber            : 8;  //最后一个分段的号码

	std::vector<TsPatProgram> program;
	uint8_t reserved_3                        : 3; // 保留位
	unsigned networkPid                    : 13; //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID

	unsigned crc_32                            : 32;  //CRC32校验码
} TsPatTable;

struct PesTime {
    PesTime() : pts(0), dts(0) {}
    int64_t pts;
    int64_t dts;
};

typedef struct TsPmtStream    
{    
	unsigned streamType                       : 8; //指示特定PID的节目元素包的类型。该处PID由elementary PID指定    
	unsigned elementaryPid                    : 13; //该域指示TS包的PID值。这些TS包含有相关的节目元素    
	unsigned esInfoLenght                    : 12; //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	uint8_t  vdataLength;
	unsigned descriptor;    
}TsPmtStream; 

//PMT 表结构体  
typedef struct TsPmtTable  
{  
	unsigned tableId                        : 8; //固定为0x02, 表示PMT表  
	unsigned sectionSyntaxIndicator        : 1; //固定为0x01  
	unsigned zero                            : 1; //0x01  
	unsigned reserved_1                      : 2; //0x03  
	unsigned sectionLength                  : 12;//首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。  
	unsigned programNumber                    : 16;// 指出该节目对应于可应用的Program map PID  
	unsigned reserved_2                        : 2; //0x03  
	unsigned versionNumber                    : 5; //指出TS流中Program map section的版本号  
	unsigned currentNextIndicator            : 1; //当该位置1时，当前传送的Program map section可用；  
	//当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。  
	unsigned sectionNumber                    : 8; //固定为0x00  
	unsigned lastSectionNumber            : 8; //固定为0x00  
	unsigned reserved_3                        : 3; //0x07  
	unsigned pcrPid                        : 13; //指明TS包的PID值，该TS包含有PCR域，  
	//该PCR值对应于由节目号指定的对应节目。  
	//如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。  
	unsigned reserved_4                        : 4; //预留为0x0F  
	unsigned programInfoLength            : 12; //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。  

	std::vector<TsPmtStream> pmtStream;  //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定  
	unsigned reserved_5                        : 3; //0x07  
	unsigned reserved_6                        : 4; //0x0F  
	unsigned crc_32                            : 32;   
} TsPmtTable;



#define TS_PACKET_SIZE  188
class Mpegts
{
public:
	Mpegts(void);
	~Mpegts(void);

	void test();
	
	int parsePat(TsPatTable *packet, unsigned char * buffer);
	int parserPmt(TsPmtTable *pmt, uint8_t *data);

    void testPtsDts();

    void parserAllPackets(uint8_t *data, int dataSize);
    bool parserPacketPts(uint8_t *data, PesTime &tm);
    int64_t  getPts(uint8_t *data);

	int makeTable(uint32_t *crc32_table);
	uint32_t crc32Calculate(const uint8_t *buffer, uint32_t size,const uint32_t *crc32_table);
public:
	uint8_t mData[188];

    int mPacketIndex;
    std::map<int64_t, int64_t> mPesTime;
    uint8_t mCurrentTsPacket[TS_PACKET_SIZE];
    int mCurrentBufferSize;

	uint8_t mPmtData[188];
	uint32_t mCrcTable[256];
};

