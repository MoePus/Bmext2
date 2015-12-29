#pragma once
#include "dec\decode.h"

#include "bstruct.h"
#include "iostream"
#include <string>
#include <locale>
#include <codecvt>
using namespace std;
class dechandle
{
public:
	dechandle(string idx, BYTE* _comBuf, BYTE* _decBuf);
	~dechandle();
	wstring getComment(DWORD bins, DWORD cids);
private:
	void getBin(int n);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	string binPrefix;
	BYTE* comBuf;
	BYTE* decBuf;

	DWORD n;

};

dechandle::dechandle(string idx, BYTE* _comBuf, BYTE* _decBuf)
{
	binPrefix = idx.substr(0, idx.length() - 4);
	ZeroMemory(_comBuf, blockSize);
	ZeroMemory(_decBuf, blockSize);
	comBuf = _comBuf;
	decBuf = _decBuf;
	n = ~0;
}

dechandle::~dechandle()
{
}

void dechandle::getBin(int n)
{
	char nTmp[20] = { 0 };
	_itoa(n, nTmp, 10);
	string binName = binPrefix + nTmp + ".bin";
	HANDLE HFile = CreateFile(binName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	DWORD rb = 0;
	DWORD compressedSize = GetFileSize(HFile, 0);
	size_t decompressedSize = blockSize;


	ReadFile(HFile, comBuf, compressedSize, &rb, 0);
	CloseHandle(HFile);
	BrotliDecompressBuffer(compressedSize, comBuf, &decompressedSize, decBuf);

}

wstring dechandle::getComment(DWORD bins, DWORD cids)
{
	if (bins != n)
	{
		getBin(bins);
		n = bins;
	}
	DWORD lp = 0;
	if (cids != 0)
	{
		lp = *(DWORD*)(decBuf + blockSize - 4 * cids);
	}

	std::wstring wideStr = conv.from_bytes((char*)(lp + decBuf));
	return wideStr;
}