#include "stdafx.h"
//brotli:Include this before anyone else
#include "dec\decode.h"
//noraml headers
#include "bread.h"
#include "breadInner.h"
#include "bstruct.h"
#include "iostream"
#include <unordered_map>
using namespace std;

const auto CRCPOLYNOMIAL = 0xEDB88320;

DWORD crctable[256] = { 0 };
void create_table()
{

	register DWORD crcreg;
	int i, j;
	for (i = 0; i < 256; ++i)
	{
		crcreg = i;
		for (j = 0; j < 8; ++j)
		{
			if ((crcreg & 1) != 0)
			{
				crcreg = CRCPOLYNOMIAL ^ (crcreg >> 1);
			}

			else
			{
				crcreg >>= 1;
			}
		}
		crctable[i] = crcreg;
	}
}
DWORD crc32(char *input, register int len)
{

	register DWORD crcstart = 0xFFFFFFFF;
	for (register int i = 0; i < len; ++i)
	{
		DWORD index = (crcstart ^ (unsigned char)*(input++)) & 0xff;
		crcstart = (crcstart >> 8) ^ crctable[index];
	}
	return crcstart ^ 0xFFFFFFFF;
}


void readBins(std::vector<bFile> fileLst, string mid)
{
	create_table();
	DWORD usrHash = crc32((char*)mid.c_str(), mid.length());
	setlocale(LC_ALL, "");
	ios_base::sync_with_stdio(false); // 缺少的话，wcout wchar_t 会漏掉中文
	wcout.imbue(locale(locale(), "", LC_CTYPE));
	wcout << L"cid,time,string" << endl;

	for (auto idx : fileLst)
	{

		HANDLE HFile = CreateFile(idx.Name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		DWORD rb = 0;
		//BYTE* decBuff = (BYTE*)malloc(idx.size);
		DWORD usrCount = 0;
		ReadFile(HFile, &usrCount, 4, &rb, 0);
		
		SetFilePointer(HFile, 4, 0, FILE_BEGIN);
		BYTE* decBuff = (BYTE*)malloc(usrCount * 8);
		ReadFile(HFile, decBuff, usrCount * 8, &rb, 0);
		BYTE* currentLp = decBuff;


		DWORD sum = 0;
		DWORD vs = 0;
		for (DWORD i = 0; i < usrCount; i++)
		{
			vs = *(DWORD*)(currentLp + 4);
			DWORD hash = *(DWORD*)(currentLp);
			if (hash == usrHash)
			{
				break;
			}
			sum += vs;
			currentLp += 8;
		}
		free(decBuff);
		if (currentLp - decBuff >= usrCount * 8) continue;

		decBuff = (BYTE*)malloc(vs * 16);
		SetFilePointer(HFile, 4 + usrCount * 8 + sum * 16, 0, FILE_BEGIN);
		ReadFile(HFile, decBuff, vs * 16, &rb, 0);
		CloseHandle(HFile);

		vector<bComment> commentLst;
		for (DWORD j = 0; j < vs; j++)
		{
			BYTE* lp = decBuff + j * 16;
			commentLst.push_back(bComment(*(DWORD*)lp, *(DWORD*)(lp + 4), *(DWORD*)(lp + 8), *(DWORD*)(lp + 12)));
		}
		free(decBuff);


		BYTE* comBuf = (BYTE*)malloc(blockSize);
		BYTE* decBuf = (BYTE*)malloc(blockSize);
		dechandle* h = new dechandle(idx.Name, comBuf, decBuf);
		for (auto comment : commentLst)
		{
			wstring cStr = h->getComment(comment.bins, comment.cids);

			wchar_t unixTime[32] = { 0 };
			time_t uT = comment.time;
			tm Utm;
			localtime_s(&Utm, &uT);
			wcsftime(unixTime, 32, L"%Y-%m-%d %H:%M:%S", &Utm);

			wcout << comment.cid << L"," << unixTime << L"," << cStr<<endl;
		}
		delete h;

		free(comBuf);
		free(decBuf);
	}
	


	return;
}


