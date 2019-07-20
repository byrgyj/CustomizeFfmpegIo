#pragma once
#define TS_PACKET_SIZE 188
#include <stdint.h>
#include <vector>
#include <map>

typedef struct TsPatProgram
{
	unsigned program_number    :16; //��Ŀ��
	unsigned program_map_PID   :13;   //��Ŀӳ����PID����Ŀ�Ŵ���0ʱ��Ӧ��PID��ÿ����Ŀ��Ӧһ��
}TsPatProgram;

//PAT��ṹ��
typedef struct TsPatTable
{
	uint8_t tableId                        : 8; //�̶�Ϊ0x00 ����־�Ǹñ���PAT
	uint8_t sectionSyntaxIndicator        : 1; //���﷨��־λ���̶�Ϊ1
	uint8_t zero                            : 1; //0
	uint8_t reserved_1                        : 2; // ����λ
	unsigned sectionLength                    : 12; //��ʾ����ֽں������õ��ֽ���������CRC32
	unsigned transportStreamId            : 16; //�ô�������ID��������һ��������������·���õ���
	uint8_t reserved_2                        : 2;// ����λ
	uint8_t versionNumber                    : 5; //��Χ0-31����ʾPAT�İ汾��
	uint8_t currentNextIndicator            : 1; //���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч
	uint8_t sectionNumber                    : 8; //�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
	uint8_t lastSectionNumber            : 8;  //���һ���ֶεĺ���

	std::vector<TsPatProgram> program;
	uint8_t reserved_3                        : 3; // ����λ
	unsigned networkPid                    : 13; //������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID

	unsigned crc_32                            : 32;  //CRC32У����
} TsPatTable;

struct PesTime {
    PesTime() : pts(0), dts(0) {}
    int64_t pts;
    int64_t dts;
};

typedef struct TsPmtStream    
{    
	unsigned streamType                       : 8; //ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��    
	unsigned elementaryPid                    : 13; //����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��    
	unsigned esInfoLenght                    : 12; //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��
	uint8_t  vdataLength;
	unsigned descriptor;    
}TsPmtStream; 

//PMT ��ṹ��  
typedef struct TsPmtTable  
{  
	unsigned tableId                        : 8; //�̶�Ϊ0x02, ��ʾPMT��  
	unsigned sectionSyntaxIndicator        : 1; //�̶�Ϊ0x01  
	unsigned zero                            : 1; //0x01  
	unsigned reserved_1                      : 2; //0x03  
	unsigned sectionLength                  : 12;//������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC��  
	unsigned programNumber                    : 16;// ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID  
	unsigned reserved_2                        : 2; //0x03  
	unsigned versionNumber                    : 5; //ָ��TS����Program map section�İ汾��  
	unsigned currentNextIndicator            : 1; //����λ��1ʱ����ǰ���͵�Program map section���ã�  
	//����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��  
	unsigned sectionNumber                    : 8; //�̶�Ϊ0x00  
	unsigned lastSectionNumber            : 8; //�̶�Ϊ0x00  
	unsigned reserved_3                        : 3; //0x07  
	unsigned pcrPid                        : 13; //ָ��TS����PIDֵ����TS������PCR��  
	//��PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ��  
	//�������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��  
	unsigned reserved_4                        : 4; //Ԥ��Ϊ0x0F  
	unsigned programInfoLength            : 12; //ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����  

	std::vector<TsPmtStream> pmtStream;  //ÿ��Ԫ�ذ���8λ, ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��  
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

