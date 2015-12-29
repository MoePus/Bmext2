#include "stdafx.h"
//brotli:Include this before anyone else
#include "enc\encode.h"
//noraml headers
#include "bwrite.h"
#include "bstruct.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <vector>
#include "time.h"
using namespace std;

class texthandle
{
public:
	texthandle(char* _randStr, DWORD _blockSize);
	~texthandle();

	inline std::pair<DWORD, DWORD> addText2bin(char* text, DWORD len)
	{
		if (len > blockSize - 16)
			len = 16;
		if (used + len + 4 <= blockSize)
		{
			used += (len + 4);
			DWORD lastlp = 0;
			if (cids != 0)
			{
				lastlp = *(DWORD*)(src + blockSize - 4 * cids);
			}
			memcpy(src + lastlp, text, len);
			cids++;
			*(DWORD*)(src + blockSize - 4 * cids) = lastlp + len;
		}
		else
		{
			compressHandle();
			return addText2bin(text, len);
		}
		return std::pair<DWORD, DWORD>(bins, cids - 1);
	}


private:
	DWORD compressHandle()
	{
		size_t compressedSize = blockSize;
		BrotliCompressBuffer(params, blockSize, (const uint8_t*)src, &compressedSize, dst);
		ZeroMemory(src, blockSize);
		char path[31] = { 0 };
		sprintf_s(path, "bin\\%s%d.bin", randStr.c_str(), bins);
		bins++;
		cids = 0;
		used = 0;
		HANDLE HFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
		DWORD wb = 0;
		WriteFile(HFile, dst, compressedSize, &wb, 0);
		CloseHandle(HFile);
		ZeroMemory(dst, blockSize);
		return compressedSize;
	}

	brotli::BrotliParams params;
	DWORD bins;
	DWORD cids;

	DWORD blockSize;
	DWORD used;
	BYTE* src;
	BYTE* dst;

	string randStr;
};

char *rand_str(char *str, const int len)
{
	int i;
	srand(time(0));
	for (i = 0; i<len; ++i)
		str[i] = 'A' + rand() % 26;
	str[++i] = '\0';
	return str;
}

void mkBins(std::vector<bFile> fileLst)
{
	char randStr[16] = {0};
	rand_str(randStr, 9);

	DWORD maxSize = 0;
	for_each(fileLst.begin(), fileLst.end(), [&maxSize](bFile f) {
		if (f.size > maxSize)
			maxSize = f.size;
	});

	BYTE* prxBuffer = (byte*)malloc(maxSize);
	ZeroMemory(prxBuffer, maxSize);
	std::unordered_map<	DWORD,
						vector<bComment> >usrMap;
	texthandle* tH=new texthandle(randStr, blockSize);
	for (DWORD i = 0; i < fileLst.size(); i++)
	{
		HANDLE HFile = CreateFileA(fileLst[i].Name,GENERIC_READ,FILE_SHARE_READ| FILE_SHARE_WRITE,0, OPEN_EXISTING,0,0);
		DWORD rb = 0;
		ReadFile(HFile, prxBuffer, fileLst[i].size, &rb, 0);
		BYTE* pointer = prxBuffer;
		while(pointer- prxBuffer<= fileLst[i].size)
		{
			DWORD hash = *(DWORD*)pointer;
			
			DWORD len = strlen((char*)pointer + 0xC) + 1;
			std::pair<DWORD, DWORD> result = tH->addText2bin((char*)pointer + 0xC, len);

			usrMap[hash].push_back(bComment(*(DWORD*)(pointer + 4), *(DWORD*)(pointer + 8), result.first, result.second));

			pointer += 0xC;
			pointer += len;
		}
		ZeroMemory(prxBuffer, fileLst[i].size);
		CloseHandle(HFile);
		if (i % 200 == 0)
			cout << i <<endl;
	}
	delete tH;
	fileLst.clear();
	vector<bFile>().swap(fileLst);

	DWORD mapSize = 4;
	for_each(usrMap.begin(), usrMap.end(), [&mapSize](auto f) {
		mapSize += 8;
		mapSize += f.second.size()*sizeof(bComment);
	});
	
	BYTE* mapBuff = (BYTE*)malloc(mapSize);
	BYTE* mapBuffCompress = (BYTE*)malloc(mapSize);
	*(DWORD*)(mapBuff) = usrMap.size();
	DWORD currentLp = 4;
	/*format
	0			4			8			C
	userhash	num			userhash	num
	*/
	for (auto kv : usrMap)
	{
		*(DWORD*)(mapBuff + currentLp) = kv.first;
		currentLp += 4;
		*(DWORD*)(mapBuff + currentLp) = kv.second.size();
		currentLp += 4;
	}
	/*format
	0			4			8			C
	cid			time		bins		cids
	*/
	for (auto kv : usrMap)
	{
		for (auto kv : kv.second)
		{
			memcpy(mapBuff + currentLp, &kv, sizeof(bComment));
			currentLp += sizeof(bComment);
		}
	}
	/*size_t compressedSize = mapSize;
	BYTE* compedBuf = (BYTE*)malloc(mapSize); 
	ZeroMemory(compedBuf, mapSize);
	brotli::BrotliParams params;
	params.quality = 1;
	params.lgblock = 24;
	BrotliCompressBuffer(params, mapSize, (const uint8_t*)mapBuff, &compressedSize, compedBuf);*/
	char path[31] = { 0 };
	sprintf_s(path, "bin\\%s.idx", randStr);
	HANDLE HFile = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
	DWORD wb = 0;
	//WriteFile(HFile, &mapSize, 4, &wb, 0);
	//SetFilePointer(HFile, 4, 0, FILE_BEGIN);
	WriteFile(HFile, mapBuff, mapSize, &wb, 0);
	CloseHandle(HFile);
	//free(compedBuf);
	free(mapBuff);
}



texthandle::texthandle(char* _randStr,DWORD _blockSize = 128*1024*1024)
{
	randStr = _randStr;
	bins = 0; cids = 0; used = 0;

	blockSize = _blockSize;
	src = (byte*)malloc(blockSize);
	dst = (byte*)malloc(blockSize);
	ZeroMemory(src, blockSize);
	ZeroMemory(dst, blockSize);

	
	params.mode = brotli::BrotliParams::Mode::MODE_TEXT;
	params.quality = 4;
	params.lgwin = 24;
}

texthandle::~texthandle()
{
	if (cids != 0)
	{
		compressHandle();
	}
	free(src);
	free(dst);
}