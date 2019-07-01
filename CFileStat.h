#ifndef __CFILESTAT_H__
#define __CFILESTAT_H__
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <map>
#include <memory>
#include <fstream>
#include <iostream>

#define DEFAULT_DRIVE "C:"
#define DEFAULT_DRIVE_LOWER "c:"

// #ifndef MAX_PATH
// #define MAX_PATH 260
// #endif

#ifndef _WIN64
#define MAXMEMORYSIZE 1024*1024*1024 //1GB
#else
#define MAXMEMORYSIZE 1024*1024*1024*10 //10GB
#endif 

enum enFileType 
{
	FILETYPE_UNKNOW = -1,
	FILETYPE_FILE,
	FILETYPE_DIR,
	FILETYPE_LINK
};

class CBase 
{
public:
	CBase(CBase* pParent, std::string strName, time_t tTime, int nFileType = FILETYPE_UNKNOW)
		: m_pParent(pParent)
		, m_strName(strName)
		, m_nCTime(tTime)
		, m_nFileType(nFileType)
	{

	}

	virtual ~CBase() 
	{
	
	}

	std::string GetTimeStr()
	{
		struct tm tm;
		localtime_s(&tm, &m_nCTime);
		char szTime[64] = { 0 };
		sprintf_s(szTime, _countof(szTime), "%04d/%02d/%02d  %02d:%02d"
			, tm.tm_year + 1900
			, tm.tm_mon + 1
			, tm.tm_mday
			, tm.tm_hour
			, tm.tm_min
			);
		return szTime;
	}

	std::string GetAbsolutePath() 
	{
		m_strAbsolutePath = "";
		if (this == m_pParent)
		{
			m_strAbsolutePath = m_strName;
			return m_strAbsolutePath;
		}
		
		CBase* pTmp = this;
		while (pTmp && pTmp != pTmp->m_pParent)
		{
			std::string strTmp = pTmp->GetName();
			m_strAbsolutePath = "\\" + pTmp->GetName() + m_strAbsolutePath;
			pTmp = pTmp->m_pParent;
		}

		std::string strTmp = pTmp->GetName();
		m_strAbsolutePath = pTmp->GetName() + m_strAbsolutePath;

		return m_strAbsolutePath;
	}

	void SetAbsolutePath(std::string strAbsolutePath)
	{
		m_strAbsolutePath = strAbsolutePath;
	}

	std::string GetName() 
	{
		return m_strName;
	}

	void SetName(std::string strName) 
	{
		m_strName = strName;
	}

	int GetFileType() 
	{
		return m_nFileType;
	}

	void SetFileType(int nFileType) 
	{
		m_nFileType = nFileType;
	}

	time_t GetTime() 
	{
		return m_nCTime;
	}

	void SetTime(time_t nTime) 
	{
		m_nCTime = nTime;
	}

	void SetParent(CBase* pParent) 
	{
		if (pParent)
			m_pParent = pParent;
	}

public:
	CBase* m_pParent;
	std::string m_strAbsolutePath;

protected:
	std::string m_strName;
	int m_nFileType;
	time_t m_nCTime;
};

class CFile : public CBase 
{
public:
	CFile(CBase* pParent, std::string strName, time_t tTime, unsigned int nSize, void* pData)
		: CBase(pParent, strName, tTime, FILETYPE_FILE)
		, m_nSize(nSize)
		, m_pData(nullptr)
	{
		if (nSize > 0 && pData !=  nullptr)
		{
			try
			{
				m_pData = new char[nSize];
				if (!m_pData)
					throw "";
			}
			catch (...)
			{
				std::cout << "out of memory" << std::endl;
				exit(0);
			}
			memcpy(m_pData, pData, nSize);
		}
	}

	~CFile() 
	{
		if (m_pData)
			delete[] m_pData, m_pData = nullptr;
	}

	unsigned int GetSize() 
	{
		return m_nSize;
	}

	void SetSize(unsigned int size) 
	{
		m_nSize = size;
	}

	char* GetData() 
	{
		return m_pData;
	}

	void SetData(char* data, unsigned int size) 
	{
		try
		{
			m_pData = new char[size];
			if (!m_pData)
				throw "";
		}
		catch (...)
		{
			std::cout << "out of memory" << std::endl;
			exit(0);
		}
		memcpy(m_pData, data, size);
		m_nSize = size;
	}

public:
	char* m_pData;
	unsigned int m_nSize;
};

class CLink : public CBase 
{
public:
	CLink(CBase* pParent, std::string strName, time_t tTime) 
		: CBase(pParent, strName, tTime, FILETYPE_LINK)
	{
		
	}

	~CLink() 
	{
	
	}

public:
	std::string GetLinkName() 
	{
		return m_strLinkName;
	}

	void SetLinkName(std::string strLinkName)
	{
		m_strLinkName = strLinkName;
	}

	std::string GetAbsoluteLinkPath()
	{
		std::string strAbsolteLinkPath;

		std::string strAbsolute = GetAbsolutePath();
		size_t nPos = std::string::npos;
		size_t nPos1 = strAbsolute.find_last_of('\\');
		if (nPos1 != std::string::npos)
		{
			nPos = nPos1;
		}
		size_t nPos2 = strAbsolute.find_last_of('/');
		if (nPos2 != std::string::npos && nPos2 > nPos1)
		{
			nPos = nPos2;
		}
		if (nPos != std::string::npos)
		{
			strAbsolute.erase(nPos + 1, strAbsolute.length() - nPos);
			strAbsolteLinkPath = strAbsolute + m_strLinkName;
		}

		return strAbsolteLinkPath;
	}

public:
	//命令的路径值即可，可能是绝对路径也可能是相对路径
	std::string m_strLinkName;
};

class CDir : public CBase 
{
public:
	CDir(CBase* pParent, std::string strName, time_t tTime)
		: CBase(pParent, strName, tTime, FILETYPE_DIR)
	{
	
	}

	~CDir() 
	{
		for (auto ele : m_mapChild)
		{
			delete ele.second, ele.second = nullptr;
		}
		m_mapChild.erase(m_mapChild.begin(), m_mapChild.end());
	}

	std::map<std::string, CBase*>& GetChild() 
	{
		return m_mapChild;
	}

public:
	std::map<std::string, CBase*> m_mapChild;
};

#endif
