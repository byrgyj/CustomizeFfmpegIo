#include "StdAfx.h"
#include "Mpegts.h"


Mpegts::Mpegts(void) : mCurrentBufferSize(0), mPacketIndex(0) {
	memset(mPmtData, 0xFF, sizeof(mPmtData));
	makeTable(mCrcTable);
}


Mpegts::~Mpegts(void)
{
}

void Mpegts::test(){
	uint8_t patData[TS_PACKET_SIZE] = {
		0x47, 0x40, 0x00, 0x10, 0x00, 0x00, 0xB0, 0x0D, 0x00, 0x01, 0xC1, 0x00, 0x00, 0x00, 0x01, 0xF0, 0x00, 
		0x2A, 0xB1, 0x04, 0xB2 
	};

	uint8_t pmtData[] = {
		0x47, 0x50, 0x00, 0x10, 0x00, 0x02, 0xB0, 0x3C, 0x00, 0x01, 0xC1, 0x00, 0x00, 0xE1, 0x00, 0xF0, 0x00, 0x06, 0xE1, 0x00,
		0xF0, 0x1B, 0x05, 0x04, 0x44, 0x4F, 0x56, 0x49, 0x38, 0x0D, 0x02, 0x20, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x78, 0xF0, 0xB0, 0x04, 0x01, 0x00, 0x0A, 0x25, 0x87, 0xE1, 0x01, 0xF0, 0x0A, 0xCC, 0x08, 0xC0, 0xC4, 0x90, 0x75,
		0x6E, 0x64, 0x01, 0x10,
		0xD2, 0xFE, 0x59, 0x70 
	};
	std::string strPmtData((char*)pmtData);

	TsPatTable pat;
	//int pmtPid = parsePat(&pat, patData + 5);

	TsPmtTable pmt;
	parserPmt(&pmt, pmtData + 5);

	memcpy(mPmtData, pmtData, pmt.pmtStream[0].vdataLength + 5);
	uint8_t * p = mPmtData;


	uint32_t crc = crc32Calculate(mPmtData + 4, pmt.pmtStream[0].vdataLength + 1, mCrcTable);

	uint8_t v1 = (crc >> 24) & 0xFF;
	uint8_t v2 = (crc >> 16) & 0xFF;
	uint8_t v3 = (crc >> 8) & 0xFF;
	uint8_t v4 = crc & 0xFF;

	int index = pmt.pmtStream[0].vdataLength + 1 + 4;
	mPmtData[index] = v1;
	mPmtData[index + 1] = v2;
	mPmtData[index + 2] = v3;
	mPmtData[index + 3] = v4;

	return;



	//uint8_t data[TS_PACKET_SIZE * 512] = { 0  };
	//memcpy(data, patData, TS_PACKET_SIZE);
	//memcpy(data + TS_PACKET_SIZE, pmtData, TS_PACKET_SIZE);
	FILE *fileSrc = fopen("./dolby_vision/2.ts", "rb");
	if (fileSrc == NULL){
		return ;
	}

	FILE *fileDest = fopen("./dolby_vision/2.265ts", "wb");
	if (fileDest == NULL){
		return;
	}


	int pmtPid = -1;
	while(true){
		uint8_t data[TS_PACKET_SIZE * 512] = { 0 };
		int len = fread(data, 1, sizeof(data), fileSrc);
		if (len <=0 ){
			break;
		}

		for (int i = 0; i < len; i+=TS_PACKET_SIZE){
			uint8_t *index = data + i;
			int pid = ((index[1] & 0x1F) << 8) | index[2];
			if (pid == 0){
				pmtPid = parsePat(&pat, index + 5);
			} else if (pmtPid == pid){
				memcpy(index + 4, pmtData + 4, sizeof(pmtData) - 4);
			}
		}

		int saveSize = fwrite(data, 1, len, fileDest);
		printf("");
	}

	if (fileSrc!=NULL){
		fclose(fileSrc);
	}
	if (fileDest != NULL){
		fclose(fileDest);
	}
}

int Mpegts::parsePat( TsPatTable * packet, unsigned char * buffer)
{
	int pmtID = -1;
	packet->tableId                    = buffer[0];
	packet->sectionSyntaxIndicator    = buffer[1] >> 7;
	packet->zero                        = buffer[1] >> 6 & 0x1;
	packet->reserved_1                    = buffer[1] >> 4 & 0x3;
	packet->sectionLength                = (buffer[1] & 0x0F) << 8 | buffer[2]; 

	packet->transportStreamId            = buffer[3] << 8 | buffer[4];

	packet->reserved_2                    = buffer[5] >> 6;
	packet->versionNumber                = buffer[5] >> 1 &  0x1F;
	packet->currentNextIndicator        = (buffer[5] << 7) >> 7;
	packet->sectionNumber                = buffer[6];
	packet->lastSectionNumber            = buffer[7];

	int len = 0;
	len = 3 + packet->sectionLength;
	packet->crc_32                        = (buffer[len-4] & 0x000000FF) << 24
		| (buffer[len-3] & 0x000000FF) << 16
		| (buffer[len-2] & 0x000000FF) << 8 
		| (buffer[len-1] & 0x000000FF); 


	int n = 0;
	for ( n = 0; n < packet->sectionLength - 12; n += 4 )
	{
		unsigned  program_num = buffer[8 + n ] << 8 | buffer[9 + n ];  
		packet->reserved_3                = buffer[10 + n ] >> 5; 

		packet->networkPid = 0x00;
		if ( program_num == 0x00)
		{  
			packet->networkPid = (buffer[10 + n ] & 0x1F) << 8 | buffer[11 + n ];
			unsigned pid = packet->networkPid;
		}
		else
		{
			TsPatProgram PAT_program;
			PAT_program.program_map_PID = (buffer[10 + n] & 0x1F) << 8 | buffer[11 + n];
			PAT_program.program_number = program_num;
			packet->program.push_back( PAT_program );

			pmtID = PAT_program.program_map_PID;
		}         
	}
	return pmtID;
}

int Mpegts::parserPmt(TsPmtTable *packet, uint8_t *buffer)  
{   
	packet->tableId                            = buffer[0];  
	packet->sectionSyntaxIndicator            = buffer[1] >> 7;  
	packet->zero                                = buffer[1] >> 6 & 0x01;   
	packet->reserved_1                            = buffer[1] >> 4 & 0x03;  
	packet->sectionLength                        = (buffer[1] & 0x0F) << 8 | buffer[2];      
	packet->programNumber                        = buffer[3] << 8 | buffer[4];  
	packet->reserved_2                            = buffer[5] >> 6;  
	packet->versionNumber                        = buffer[5] >> 1 & 0x1F;  
	packet->currentNextIndicator                = (buffer[5] << 7) >> 7;  
	packet->sectionNumber                        = buffer[6];  
	packet->lastSectionNumber                    = buffer[7];  
	packet->reserved_3                            = buffer[8] >> 5;  
	packet->pcrPid                                = ((buffer[8] << 8) | buffer[9]) & 0x1FFF;  

	int PCRID = packet->pcrPid;  

	packet->reserved_4                            = buffer[10] >> 4;  
	packet->programInfoLength                    = (buffer[10] & 0x0F) << 8 | buffer[11];   
	// Get CRC_32  
	int len = 0;  
	len = packet->sectionLength + 3;      
	packet->crc_32                = (buffer[len-4] & 0x000000FF) << 24  
		| (buffer[len-3] & 0x000000FF) << 16  
		| (buffer[len-2] & 0x000000FF) << 8  
		| (buffer[len-1] & 0x000000FF);   

	int pos = 12;  
	// program info descriptor  
	if ( packet->programInfoLength != 0 )  
		pos += packet->programInfoLength;      
	// Get stream type and PID      
	for ( ; pos <= (packet->sectionLength + 2 ) -  4; )  
	{  
		TsPmtStream pmt_stream;  
		pmt_stream.streamType =  buffer[pos];  
		packet->reserved_5  =   buffer[pos+1] >> 5;  
		pmt_stream.elementaryPid =  ((buffer[pos+1] << 8) | buffer[pos+2]) & 0x1FFF;  
		packet->reserved_6     =   buffer[pos+3] >> 4;  
		pmt_stream.esInfoLenght =   (buffer[pos+3] & 0x0F) << 8 | buffer[pos+4];
		pmt_stream.vdataLength = pmt_stream.esInfoLenght + pos + 5;

		pmt_stream.descriptor = 0x00;  
		if (pmt_stream.esInfoLenght != 0)  
		{  
			pmt_stream.descriptor = buffer[pos + 5];  

// 			for( int len = 2; len <= pmt_stream.esInfoLenght; len ++ )  
// 			{  
// 				pmt_stream.descriptor = pmt_stream.descriptor<< 8 | buffer[pos + 4 + len];  
// 			}  
			pos += pmt_stream.esInfoLenght;  
		}  
		pos += 5;  
		packet->pmtStream.push_back(pmt_stream);  
	}  
	return 0;  
}

void Mpegts::testPtsDts() {
    //FILE *f = fopen("./dc36d88ec45cb13e236bb149dde5b76d (2).ts", "rb");
    FILE *f = fopen("./bd5cbc627975456aa65eb3eb6ff8c678.ts", "rb");
    if (f == NULL) {
        return;
    }

    int bufferSize = 32*1024;
    int indexArray[] = { 40, 100, 50,  32* 1024, 10*1024, 888, 25000 };
    int arrayCount = _countof(indexArray);
    uint8_t *buffer = new uint8_t[bufferSize];
    int index = 0;
    while(!feof(f)){
        int length = fread(buffer, 1, indexArray[index%arrayCount], f);
        parserAllPackets(buffer, length);
        index++;
    }

    if (buffer != NULL) {
        delete buffer;
    }

    for (std::map<int64_t, int64_t>::iterator it = mPesTime.begin(); it != mPesTime.end(); it++) {
        printf ("dts:%lld, pts:%lld \n", it->second, it->first);
    }

}

void Mpegts::parserAllPackets(uint8_t *data, int dataSize) {
    if (data == NULL || dataSize <= 0) {
        return ;
    }



    if (dataSize + mCurrentBufferSize < TS_PACKET_SIZE) {
        memcpy(mCurrentTsPacket + mCurrentBufferSize, data, dataSize);
        mCurrentBufferSize += dataSize;
        return;
    }

    int index = 0;
    int totalSize = dataSize + mCurrentBufferSize;
    int leftData = totalSize;

    if (mCurrentBufferSize == 0) {

    } else {
        memcpy(mCurrentTsPacket + mCurrentBufferSize, data, TS_PACKET_SIZE - mCurrentBufferSize);

       PesTime pt;
       uint8_t *curPacket = mCurrentTsPacket;
       //printf("binary data print1 [%x, %x, %x, %x, %x, %x]", mCurrentTsPacket[0], mCurrentTsPacket[1], mCurrentTsPacket[2], mCurrentTsPacket[3], mCurrentTsPacket[4], mCurrentTsPacket[5]);
        if (parserPacketPts(mCurrentTsPacket, pt)){
            printf("parse pts, pts:%lld, dts:%lld", pt.pts, pt.dts);
            mPesTime.insert(std::make_pair(pt.dts, pt.pts));
        } else {

        }
        mPacketIndex++;
        index = TS_PACKET_SIZE - mCurrentBufferSize;
        leftData -= TS_PACKET_SIZE;
    }

    uint8_t *curData = NULL; 
    while (index < totalSize) {
        curData = data + index;
        if (leftData >= TS_PACKET_SIZE) {
            PesTime pt;
            //printf("binary data print2 [%x, %x, %x, %x, %x, %x]", curData[0], curData[1], curData[2], curData[3], curData[4], curData[5]);
            if (parserPacketPts(curData, pt)){
                printf("parse pts, pts:%lld, dts:%lld", pt.pts, pt.dts);
                mPesTime.insert(std::make_pair(pt.dts, pt.pts));
            } else {
                printf("");
            }

            mPacketIndex++;
            leftData -= TS_PACKET_SIZE;
            index += TS_PACKET_SIZE;
        } else {
            curData = data + index;
            break;
        }
    }

    curData = data + index; 
    memset(mCurrentTsPacket, 0, TS_PACKET_SIZE);
    if (leftData > 0) {
        memcpy(mCurrentTsPacket, data + index, leftData);
        mCurrentBufferSize = leftData;
    } else {
        mCurrentBufferSize = 0;
    }
}

bool Mpegts::parserPacketPts(uint8_t *data, PesTime &tm) {
    char sync = data[0];

    // sycn flag
    if (sync != 0x47) {
        return false;
    }

    // payload flag
    if ((data[1] & 0x40) !=  0x40){
        return false;
    }

    int adationLength = 0;
    int  adaption =  (data[3] >> 4)  &  3;
    if (adaption == 3) {
        adationLength = data[4] + 1;
    }

    int index = adationLength + 4;
    if ( !(data[index] == 0 && data[index + 1] == 0 && data[index + 2] == 1)){
        return false;
    }

    index += 3;

    if (data[index] != 0xE0) {
        return false;
    }

    index += 4; //

    int hasPts  = data[index];
    int ptsFlag = (data[index] & 0xC0) >> 6;

    index += 1;
    int pesLength = data[index];

    index += 1;
    if (ptsFlag == 2) { // pts
        tm.dts = tm.pts =  getPts(data + index);
    } else if (ptsFlag == 3) {// pts dts
          tm.pts = getPts(data +index);
          index += 5;
          tm.dts = getPts(data + index);
    }

    return true;
}

int64_t Mpegts::getPts(uint8_t *data) {
    int64_t ts_32_30 = (int64_t)(*data & 0x0e) << 29;
    int64_t ts_29_15 = ( ( (data[1] << 8) | data[2] ) >> 1) << 15;
    int64_t ts_14_0  = ( (data[3] << 8) | data[4] ) >> 1;
    int64_t ts = ts_32_30 | ts_29_15 | ts_14_0;

    return ts;
}


int Mpegts::makeTable(uint32_t *crc32_table)  
{  
	for(uint32_t i = 0; i < 256; i++ ) {  
		uint32_t k = 0;  
		for(uint32_t j = (i << 24) | 0x800000; j != 0x80000000; j <<= 1 ) {  
			k = (k << 1) ^ (((k ^ j) & 0x80000000) ? 0x04c11db7 : 0);  
		}  

		crc32_table[i] = k;  
	} 

	return 0;
}  
uint32_t Mpegts::crc32Calculate(const uint8_t *buffer, uint32_t size,const uint32_t *crc32_table)  
{  
	uint32_t crc32_reg = 0xFFFFFFFF;  
	for (uint32_t i = 0; i < size; i++) {  
		crc32_reg = (crc32_reg << 8 ) ^ crc32_table[((crc32_reg >> 24) ^ *buffer++) & 0xFF];  
	}  
	return crc32_reg;  
}  