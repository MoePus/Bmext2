// bcmExt2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//brotliHeader
//#include "dec\decode.h"
//#include "enc\encode.h"
//bcmExtHeader
#include "bread.h"
#include "bwrite.h"
#include "bstruct.h"
//defaultHeader
#include "GOP\GetOpt++.h"
#include "windows.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

std::vector<bFile> buildlist(const char * lpPath, const char* ext);

std::string ws2s(const std::wstring& ws);
std::wstring s2ws(const std::string& s);

int main()
{
	std::unordered_map<std::wstring, std::wstring> opts;
	try {
		opts = GetOpt::getOpt(L"c:p:mid", GetCommandLineW());//c是处理类别，p是处理路径
	}
	catch (GetOpt::optException exp)
	{
		wcout << exp.getException() << endl;
		return -1;
	}

	if (opts.find(L"c")==opts.end() || opts.find(L"p") == opts.end())
	{
		wcout << L"Missing options.\nCheck -c or -p" << endl;
		return -1;
	}
		

	if (opts[L"c"] == L"read")
	{
		std::vector<bFile> fileLst = buildlist(ws2s(opts[L"p"]).c_str(), ".idx");
		if (fileLst.size() == 0)
		{
			wcout << L"Wrong floder.\nCheck -p" << endl;
			return -1;
		}
		if (opts.find(L"mid") == opts.end())
		{
			wcout << L"Missing options.\nCheck --mid" << endl;
			return -1;
		}
		readBins(fileLst,ws2s(opts[L"mid"]));
	}
	else if (opts[L"c"] == L"write")
	{
		std::vector<bFile> fileLst = buildlist(ws2s(opts[L"p"]).c_str(), ".prx");
		if (fileLst.size() == 0)
		{
			wcout << L"Wrong floder.\nCheck -p" << endl;
			return -1;
		}
		mkBins(fileLst);
	}
	else
	{
		wcout << L"Wrong action.\nCheck -c" << endl;
		return -1;
	}

    return 0;
}

std::vector<bFile> buildlist(const char * lpPath, const char* ext)
{
	std::vector<bFile> FList;
	char szFind[MAX_PATH];
	WIN32_FIND_DATAA FindFileData;
	sprintf_s(szFind, "%s\\*%s", lpPath, ext);
	HANDLE hFind = ::FindFirstFileA(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		while (TRUE)
		{

			char szFile[MAX_PATH];
			sprintf_s(szFile, "%s\\%s", lpPath, FindFileData.cFileName);
			FList.push_back(bFile(szFile, FindFileData.nFileSizeLow));

			if (!FindNextFileA(hFind, &FindFileData))
				break;
		}
	}
	FindClose(hFind);

	ZeroMemory(szFind, MAX_PATH);
	sprintf_s(szFind, "%s\\*.*", lpPath);
	hFind = ::FindFirstFileA(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if (strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0)
				continue;
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char NPath[MAX_PATH] = { 0 };
				sprintf(NPath, "%s\\%s", lpPath, FindFileData.cFileName);

				std::vector<bFile> NFList;
				NFList = buildlist(NPath, ext);
				for (auto pv : NFList)
				{
					FList.push_back(pv);
				}
			}
		} while (FindNextFileA(hFind, &FindFileData));
	}



	return FList;
}

std::string ws2s(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";  
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	std::string result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

std::wstring s2ws(const std::string& s)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	std::wstring result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return result;
}