#pragma once

#include "GetOptsInner.h"
#include "iostream"
#include "unordered_map"
#include "vector"
#include "string"
using namespace std;

namespace GetOpt
{
	class optException
	{
	public:
		std::wstring getException()
		{
			return exception;
		}
		optException::optException(std::wstring &&initException)
		{
			exception = initException;
		}
		optException::~optException()
		{
			exception.clear();
		}
	private:
		std::wstring exception;
	};



	std::unordered_map<std::wstring, std::wstring> getOpt(std::wstring optsChars, std::wstring commandLine) throw (optException)
	{
		std::unordered_map<std::wstring, std::wstring> ret;
		std::vector<std::wstring> opts = GetOptInner::SplitString(optsChars.c_str(), L':');
		if (opts.size() == 0)
		{
			throw optException(L"unclosed quotation marks in opts");
		}
		std::vector<std::wstring> chrOpts = GetOptInner::buildChrString(opts);


		std::vector<std::wstring> commands = GetOptInner::SplitString(commandLine.c_str(), L' ');
		if (commands.size() % 2 != 1)
		{
			throw optException(L"wrong command line");
		}

		int errOrder = GetOptInner::buildOpts(&ret, std::move(chrOpts), std::move(opts), commands);
		if (errOrder)
		{
			throw optException(L"unknown option: " + commands[errOrder]);
		}

		ret[L"launchPath"] = commands[0];

		return ret;
	}
}
