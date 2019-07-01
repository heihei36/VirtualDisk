#include "stdafx.h"
#include "CCmdParse.h"
#include <algorithm>
#include <iostream>

CCmdParse::CCmdParse()
{

}

CCmdParse::~CCmdParse() 
{

}

void CCmdParse::LeftTrim(std::string& strBuf)
{
	if (strBuf.length() <= 0)
		return;

	for (auto index = strBuf.begin(); index < strBuf.end();)
	{
		if (*index == ' ' || *index == '\t')
		{
			index = strBuf.erase(index);
		}
		else
		{
			break;
		}
	}
}

void CCmdParse::RightTrim(std::string& strBuf)
{
	if (strBuf.length() <= 0)
		return;

	for (auto index = strBuf.end() - 1; index >= strBuf.begin();)
	{
		if (*index == ' ' || *index == '\t')
		{
			index = strBuf.erase(index);
			--index;
		}
		else
		{
			break;
		}
	}
}

void CCmdParse::TrimSpace(std::string& strBuf)
{
	LeftTrim(strBuf);
	RightTrim(strBuf);
}

int CCmdParse::FindCmdSeparator(std::string& strCmd)
{
	if (strCmd.length() <= 0)
		return -1;

	size_t m = -1;
	size_t m1 = strCmd.find_first_of(' ');
	size_t m2 = strCmd.find_first_of('\t');
	if (m1 != std::string::npos)
	{
		m = m1;
	}
	if (m2 != std::string::npos)
	{
		m = m2;
	}

	if (m1 != std::string::npos && m2 != std::string::npos)
	{
		m = std::min(m1, m2);
	}

	if (m == std::string::npos)
	{
		m = strCmd.length();
	}
	return (int)m;
}

int CCmdParse::FindSeparator(std::string& strCmd)
{
	if (strCmd.length() <= 0)
		return -1;

	size_t m = -1;
	size_t m1 = strCmd.find_first_of(' ');
	size_t m2 = strCmd.find_first_of('\t');
	size_t m3 = strCmd.find_first_of('"');
	size_t m4 = m3;
	if (m1 != std::string::npos)
	{
		m = m1;
	}
	if (m2 != std::string::npos)
	{
		m = m2;
	}
	if (m3 != std::string::npos)
	{
		m4 = strCmd.find_first_of('"', m3 + 1);
	}
	m = std::min(m1, m2);
	m = std::min(m, m3);
	if (m == std::string::npos)
	{
		m = strCmd.length();
	}

	if (m == m3)
	{
		//删除二个"号
		strCmd.erase(strCmd.begin() + m3);
		strCmd.erase(strCmd.begin() + m4-1);
		return (int)(m4 - 1);
	}

	return (int)m;
}


int CCmdParse::FindCmdType(std::string& strCmd, std::string& strCmdType)
{
	TrimSpace(strCmd);

	if (strCmd.length() <= 0)
		return -1;

	int nPos = FindCmdSeparator(strCmd);
	if (nPos < 0)
	{
		nPos = (int)strCmd.length();
	}

	strCmdType.assign(strCmd.begin(), strCmd.begin() + nPos);
	strCmd.erase(0, nPos);

	return nPos;
}

int CCmdParse::FindOptionsAndParams(std::string& strCmd
	, std::vector<std::string>& vOption
	, std::vector<std::string>& vParam)
{
	if (strCmd.length() <= 0)
		return -1;

	//先验证带'"'是否配对,若不配对在路径的最后加上'"'
	std::string strTmp = strCmd;
	while (1)
	{
		TrimSpace(strTmp);
		std::string strPath = strTmp;
		size_t nBegin = strPath.find('"', 0);
		if (nBegin == std::string::npos)
			break;
		size_t nEnd = strPath.find('"', nBegin + 1);
		if (nEnd == std::string::npos)
		{
			strCmd.append(1, '"');
			break;
		}
		strPath = strPath.substr(nBegin + 1, nEnd - nBegin - 1);
		strTmp.erase(nBegin, nEnd - nBegin + 1);
	}

	while (strCmd.length() > 0)
	{
		TrimSpace(strCmd);
		int nPos = FindSeparator(strCmd);
		if (nPos > 0)
		{
			std::string strTmp(strCmd.begin(), strCmd.begin() + nPos);
			
			if (strTmp[0] == '\\' || strTmp[0] == '/')
			{
				vOption.push_back(strTmp);
			}
			else
			{
				vParam.push_back(strTmp);
			}
			strCmd.erase(0, nPos + 1);
		}
		else
		{
			if (strCmd.length() > 0)
				vParam.push_back(strCmd);
			break;
		}
	}

	return 0;
}

int CCmdParse::Parse(std::string strCmd
	, std::string& strCmdType
	, std::vector<std::string>& vOption
	, std::vector<std::string>& vParam) 
{
	if (strCmd.length() <= 0)
		return -1;
	std::string strTmp = strCmd;

	int nRet = FindCmdType(strTmp, strCmdType);
	if (nRet < 0)
		return -1;

	FindOptionsAndParams(strTmp, vOption, vParam);
	return 0;
}
