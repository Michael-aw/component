#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
	
#define BMP_TYPE 		0x424D
#define BI_RGB 			0
#define BMP1555_BYTE 	2
#define BMP24_BYTE   	3
#define BMP32_BYTE 		4
	
#define CURBMP_BYTE 	BMP24_BYTE
	
#define BMP24_TO_16(B, G, R)   ((B << 0)|(G << 5)|(R << 10))
	
typedef unsigned short WORD;
typedef unsigned int  DWORD;
typedef unsigned long  LONG;
	
#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {	  
	WORD	bfType; 	
	DWORD	bfSize; 	
	WORD	bfReserved1;	 
	WORD	bfReserved2;	 
	DWORD	bfOffBits; 
} BITMAPFILEHEADER;

#pragma pack(1)
typedef struct tagBITMAPINFOHEADER { 
	DWORD  biSize;	   
	LONG   biWidth; 	
	LONG   biHeight;	 
	WORD   biPlanes;	 
	WORD   biBitCount;	   
	DWORD  biCompression;	  
	DWORD  biSizeImage; 	
	LONG   biXPelsPerMeter; 	
	LONG   biYPelsPerMeter; 	
	DWORD  biClrUsed;	  
	DWORD  biClrImportant; 
} BITMAPINFOHEADER; 
	
	
int main(int argc,char** argv)
{
	int nErrorCode=0;
	FILE *fin;
	FILE *fout;
	char szInFilename[256];
	char szOutFilename[256];

	if(argc==3) {
		strcpy(szInFilename ,argv[1]);
		strcpy(szOutFilename,argv[2]);
	} else {
		strcpy(szInFilename , "333.bmp");
		strcpy(szOutFilename, "444.bmp");
	}
	
	printf("src:%s, dest:%s\n",szInFilename,szOutFilename);
	fin  = fopen(szInFilename , "rb");
	fout = fopen(szOutFilename, "wb");
	
	if(fin && fout) {
		//get BufHeader   
		BITMAPFILEHEADER BmpBufHeader;						   
		if(fread(&BmpBufHeader, 1, sizeof(BITMAPFILEHEADER), fin)){ 						  
			printf("Type:  0x%x\n",BmpBufHeader.bfType);
			printf("Size:  0x%x\n",BmpBufHeader.bfSize);
			printf("Offset:%d\n"  ,BmpBufHeader.bfOffBits);  
		} else {
			nErrorCode |= 0x2;
		}
		//get InfoHeader  
		BITMAPINFOHEADER BmpInfoHeader;
		if(fread(&BmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fin)){
			printf("biWidth :	%d\n",BmpInfoHeader.biWidth); 
			printf("biHeight:	%d\n",BmpInfoHeader.biHeight);
			printf("biBitCount: %d\n",BmpInfoHeader.biBitCount);
			printf("biCompression: %d\n",BmpInfoHeader.biCompression);	
		}
		else
			nErrorCode |= 0x4;
	
		//handle Header
		int BmpDataNum;  
		if(BmpInfoHeader.biBitCount != BMP24_BYTE*8
			&& BmpBufHeader.bfType != BMP_TYPE
			&& BmpInfoHeader.biCompression != BI_RGB) {
			nErrorCode |= 0x8;	
		}
		BmpInfoHeader.biBitCount  = BMP1555_BYTE * 8;
		BmpDataNum = (BmpBufHeader.bfSize - BmpBufHeader.bfOffBits) / CURBMP_BYTE;
		BmpBufHeader.bfSize = BmpDataNum * BMP1555_BYTE * 8 + BmpBufHeader.bfOffBits; 
		BmpInfoHeader.biSizeImage = (BmpInfoHeader.biSizeImage * BMP1555_BYTE) / CURBMP_BYTE;
		
		//get BmpData
		fseek(fin, 54, SEEK_SET);
		unsigned char  *BmpDatabuff    = malloc(CURBMP_BYTE   * BmpDataNum);
		unsigned short *BmpDatabuffNew = malloc(BMP1555_BYTE * BmpDataNum);
		if(!fread (BmpDatabuff, 1, CURBMP_BYTE * BmpDataNum, fin))
			nErrorCode |= 0x10;
		memset(BmpDatabuffNew, 0, BMP1555_BYTE * BmpDataNum );
		
		//handle BmpData(important)
		int i;
		unsigned int B,G,R;
		for(i = 0; i < BmpDataNum; i++) {		 
			B = (BmpDatabuff[i*CURBMP_BYTE+0] >> 3)&0x1F;//blue
			G = (BmpDatabuff[i*CURBMP_BYTE+1] >> 3)&0x1F;//green
			R = (BmpDatabuff[i*CURBMP_BYTE+2] >> 3)&0x1F;//red
			BmpDatabuffNew[i] = (unsigned short)BMP24_TO_16(B, G, R);
		}	   
		
		//creat NewBmp
		fseek(fout, 0, SEEK_SET);
		if(!fwrite(&BmpBufHeader , 1, sizeof(BITMAPFILEHEADER), fout)){
			nErrorCode |= 0x20;
		}
		if(!fwrite(&BmpInfoHeader, 1, sizeof(BITMAPINFOHEADER), fout)){
			nErrorCode |= 0x40;
		}	 
		fseek(fout, 54, SEEK_SET);
		if(!fwrite(BmpDatabuffNew, BMP1555_BYTE, BmpDataNum, fout)) {
			nErrorCode |= 0x80;
		}					  	   
	}else{
		nErrorCode |= 0x1;
	}
	
	if(fin){
		fclose(fin);
	}
	if(fout) {
		fclose(fout);
	}
	printf("nErrorCode: %d\n", nErrorCode);
	printf("BMP32_TO_16:%s\n",!nErrorCode?"success":"fail");
	
	return nErrorCode;
}
