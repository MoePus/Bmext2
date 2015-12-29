#pragma once
#include "iostream"
#include "vector"
#include "unordered_map"
#include "string"

namespace GetOptInner
{
	std::wstring delQuo(wstring str)
	{
		if (*str.c_str() == L'\"' && *(str.c_str() + str.length() - 1) == L'\"')
		{
			str = str.substr(1, str.length() - 2);
		}
		return str;
	}

std::vector<std::wstring> SplitString(const wchar_t* opts, wchar_t delimiter)
{
	while (*opts == delimiter)
		opts++;

	std::vector<std::wstring> fragments;
	std::vector<std::pair<const wchar_t*,int>> quoLst;
	const wchar_t* preOpts = opts;
	while (auto next = wcschr(preOpts, L'\"'))
	{

		const wchar_t* t = next;
		preOpts = next + 1;
		next = wcschr(preOpts, L'\"');

		if (!next)
			return fragments;
		quoLst.push_back(std::make_pair(t,next-t));
		preOpts = next + 1;
	}

	while (auto next = wcschr(opts, delimiter))
	{
		for (int i = 0; i < quoLst.size();i++)
		{
			if (next > quoLst[i].first && next < quoLst[i].first + quoLst[i].second)
			{
				next = wcschr(next + 1, delimiter);
				
				i = -1;
			}
		}
		if (!next)
		{
			break;
		}
		else
		{
			fragments.push_back(delQuo(std::wstring(opts, next)));

			while (*next == *(next + 1))
				next++;
			opts = next + 1;
		}
	}

	if (*opts != 0)
		fragments.push_back(delQuo(std::wstring(opts)));
	return fragments;
}




std::vector<std::wstring> buildChrString(std::vector<std::wstring> opts)
{
	std::vector<std::wstring> chrString;
	for each (std::wstring var in opts)
	{
		if (var.length() > 1)
		{
			chrString.push_back(L"--" + var);
		}
		else
		{
			chrString.push_back(L"-" + var);
		}
	}
	return chrString;
}

int matchOpt(std::vector<std::wstring>& chrOpts, std::wstring command)
{
	for (int i = 0; i < chrOpts.size();i++)
	{
		if (chrOpts[i] == command)
			return i;
	}
	return -1;
}

int buildOpts(
	std::unordered_map<std::wstring, std::wstring>* building,
	std::vector<std::wstring>&& chrOpts,
	std::vector<std::wstring> opts,
	std::vector<std::wstring> commands
	)
{
	for (int i = 1; i < commands.size(); i+=2)
	{
		int n = matchOpt(chrOpts, commands[i]);
		if (n == -1)return i;
		(*building)[opts[n]] = commands[i + 1];
	}
	return 0;

}
}