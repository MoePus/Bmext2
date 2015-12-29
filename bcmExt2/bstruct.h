#pragma once
#include "windows.h"
#include <iostream>
const auto blockSize = 16*1024*1024;
struct bFile
{
	char Name[MAX_PATH];
	DWORD size;
	bFile::bFile(char* a,DWORD b)
	{
		memset(Name, 0, MAX_PATH);
		strcat_s(Name, a);
		size = b;
	}
};


struct bComment
{
	bComment::bComment(DWORD _cid, DWORD _time, DWORD _bins, DWORD _cids)
	{
		cid = _cid;
		time = _time;
		bins = _bins;
		cids = _cids;
	}
	DWORD cid;
	DWORD time;
	DWORD bins;
	DWORD cids;
};