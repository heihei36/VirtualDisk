#include "stdafx.h"
#include "CCommand.h"
#include <windows.h>
#include <regex>
#include <algorithm>
#include <io.h>
#include <fstream>
#include <exception>

CBase* CBaseCmd::GetPtrByPath(std::string strPath)
{
	CBase* pBase = nullptr;
	std::string strAbsolutePath;
	int nRet = ToAbsolutePath(strPath, strAbsolutePath);
	if (nRet < 0)
		return pBase;

	std::vector<std::string> vRoutePath;
	ParsePathRoute(strAbsolutePath, vRoutePath);

	CDir* pDir = dynamic_cast<CDir*>(GetRoot());
	//��·���ڵ�Ϊ1ʱ����ô���ظ�·��
	if (vRoutePath.size() == 1)
	{
		//pBase = pDir;
		return pDir;
	}

	//��·������Ҫ����i��1��ʼ
	size_t i = 1;
	for (; i < vRoutePath.size(); i++)
	{
		if (vRoutePath[i] == ".")
		{
			if (i == vRoutePath.size() - 1)
				pBase = m_pCur;
			continue;
		}

		if (vRoutePath[i] == "..")
		{
			if (i == vRoutePath.size() - 1)
				pBase = pDir->m_pParent;

			pDir = dynamic_cast<CDir*>(pDir->m_pParent);
			continue;
		}

		auto it = pDir->m_mapChild.find(vRoutePath[i]);
		if (it != pDir->m_mapChild.end())
		{
			//���һ��
			if (i == vRoutePath.size() - 1)
			{
				pBase = it->second;
			}

			if (it->second->GetFileType() == FILETYPE_DIR)
			{
				pDir = dynamic_cast<CDir*>(it->second);
			}
			else
			{
				break;
			}
		}
		else
			break;
	}

	return pBase;
}

int CBaseCmd::PathType(std::string strPath)
{
	if (strPath.length() <= 0)
	{
		return PATHTYPE_ERROR;
	}

	int nPathType = PATHTYPE_RELATIVE;
	int nLen = min((int)strPath.length(), (int)strlen(DEFAULT_DRIVE));
	strPath.assign(strPath.begin(), strPath.begin() + nLen);
	if (0 == strcmp(strPath.c_str(), DEFAULT_DRIVE) 
		|| 0 == strcmp(strPath.c_str(), DEFAULT_DRIVE_LOWER))
	{
		nPathType = PATHTYPE_ABSOLUTE;
	}

	return nPathType;
}

//ͳһ��ɾ���·��������
int CBaseCmd::ToAbsolutePath(std::string strPath, std::string& strAbsolutePath)
{
	int nPathType = PathType(strPath);
	switch (nPathType)
	{
	case PATHTYPE_ERROR:
		return -1;
	case PATHTYPE_ABSOLUTE:
		strAbsolutePath = strPath;
		break;
	case PATHTYPE_RELATIVE:
		strAbsolutePath = m_pCur->GetAbsolutePath() + "\\" + strPath;
		break;
	default:
		return -1;
	}

	return 0;
}

//����·��·��
int CBaseCmd::ParsePathRoute(std::string strPath, std::vector<std::string>& vPathRoute)
{
	std::string strTmp1 = strPath;
	std::string strTmp2 = strPath;
	while (true)
	{
		size_t nPos = -1;
		size_t nPos1 = strTmp1.find_first_of('\\');
		if (nPos1 >= 0)
		{
			nPos = nPos1;
		}

		size_t nPos2 = strTmp1.find_first_of('/');
		if (nPos2 >= 0)
		{
			nPos = nPos2;
		}

		if (nPos1 >= 0 && nPos2 >= 0)
		{
			nPos = min(nPos1, nPos2);
		}

		if (nPos != std::string::npos)
		{
			strTmp2.erase(0, nPos + 1);
			strTmp1.erase(nPos, strTmp1.length() - nPos);
			if (strTmp1.length() > 0 && strTmp1[0] != '\\' && strTmp1[0] != '/')
				vPathRoute.push_back(strTmp1);
			strTmp1 = strTmp2;
		}
		else
		{
			if (strTmp1.length() > 0 && strTmp1[0] != '\\' && strTmp1[0] != '/')
				vPathRoute.push_back(strTmp1);
			break;
		}
	}

	return 0;
}

CBase* CBaseCmd::GetRoot()
{
	CBase* pBase = m_pCur;
	while (pBase != pBase->m_pParent)
	{
		pBase = pBase->m_pParent;
	}

	return pBase;
}

int CBaseCmd::PathFormat(std::string& strPath) 
{
	if (strPath.length() == 0)
		return PATHFORMAT_EMPTY;
	else if (strPath[0] == '@')
	{
		if (strPath.length() == 1)
			return PATHFORMAT_ERROR;
		else 
		{
			strPath.erase(0, 1);
			return PATHFORMAT_REALPATH;
		}
	}
	else
		return PATHFORMAT_VIRTUALPATH;
}

int CBaseCmd::FileIsExist(std::string strPath, int nFileType, CBase** ppBase)
{
	int nRet = 0;
	if (nFileType == PATHFORMAT_REALPATH)
	{
		if (_access(strPath.c_str(), 6) != 0)
		{
			throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
		}
	}
	else
	{
		CBase* pBase = GetPtrByPath(strPath);
		if (!pBase)
		{
			//std::cout << "ϵͳ�Ҳ���ָ�����ļ���" << std::endl;
			nRet = -1;
		}
		if (ppBase)
		{
			*ppBase = pBase;
		}
	}

	return nRet;
}

int CBaseCmd::GetFileSize(std::string strPath, int nFileType, CBase** ppBase)
{
	int nSize = -1;
	if (nFileType == PATHFORMAT_REALPATH)
	{
		std::ifstream is(strPath);
		is.seekg(0, std::ios::end);
		nSize = static_cast<int>(is.tellg());
		is.close();
	}
	else
	{
		int nType = (*ppBase)->GetFileType();
		switch (nType)
		{
		case FILETYPE_DIR:
		case FILETYPE_LINK:
			nSize = 0;
			break;
		case FILETYPE_FILE:
		{
			CFile* pFile = dynamic_cast<CFile*>(*ppBase);
			nSize = pFile->GetSize();
		}
		break;
		case FILETYPE_UNKNOW:
		default:
			break;
		}
	}

	return nSize;
}

int CBaseCmd::GetFileData(std::string strPath, int nFileType, void* pData, int& nSize, CBase** ppBase)
{
	int nRet = -1;
	if (nFileType == PATHFORMAT_REALPATH)
	{
		std::ifstream is(strPath);
		is.read(reinterpret_cast<char*>(pData), nSize);
		nRet = 0;
		is.close();
	}
	else
	{
		int nType = (*ppBase)->GetFileType();
		switch (nType)
		{
		case FILETYPE_DIR:
		case FILETYPE_LINK:
			nRet = -1;
			break;
		case FILETYPE_FILE:
		{
			CFile* pFile = dynamic_cast<CFile*>(*ppBase);
			memcpy(pData, pFile->GetData(), nSize);
			nRet = 0;
		}
		break;
		case FILETYPE_UNKNOW:
			nRet = -1;
			break;
		default:
			nRet = -1;
			break;
		}
	}

	return nRet;
}

int CBaseCmd::FileIsDir(std::string strPath) 
{
	DWORD dwAttr = GetFileAttributes(strPath.c_str());
	if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return 0;
	else
		return 1;
}

//�ļ����п�����ͨ���,�ļ�·���в�������,ֻ֧��*��?
int CBaseCmd::GetMatchFile(std::string strPath, std::vector<std::string>& vMatchFile, int nPathFormat)
{
	int nRet = 0;
	if (strPath.length() <= 0)
		return -1;

	char drive[_MAX_DRIVE] = { 0 }, dir[_MAX_DIR] = { 0 }, fname[_MAX_FNAME] = { 0 }, ext[_MAX_EXT] = { 0 };
	_splitpath_s(strPath.c_str(), drive, dir, fname, ext);
	std::string strFileName = std::string(fname) + ext;
	std::string strFilePath = std::string(drive) + dir;
	//���·���м�û��*Ҳû��?,��ֱ�ӷ���
	size_t nPos1 = strFileName.find('*');
	size_t nPos2 = strFileName.find('?');
	if (nPos1 == std::string::npos && nPos2 == std::string::npos)
	{
		if (0 != FileIsExist(strPath, nPathFormat, nullptr))
		{
			throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
		}
		else
		{
			vMatchFile.push_back(strPath);
			return 0;
		}
	}

	if (0 != FileIsExist(strFilePath, nPathFormat, nullptr))
	{
		throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
	}

	std::string strRegexFile;
	size_t nIndex = 0;
	while (nIndex < strFileName.length())
	{
		if (strFileName[nIndex] == '*')
		{
			strRegexFile += ".*";
		}
		else if (strFileName[nIndex] == '?')
		{
			strRegexFile += ".";
		}
		else
		{
			strRegexFile += strFileName[nIndex];
		}
		nIndex++;
	}

	//����Ŀ¼,����ƥ��
	std::vector<std::string> vChild;
	GetChild(strFilePath, vChild, nPathFormat);
	std::regex reg(strRegexFile);
	std::smatch match;
	for (auto ele : vChild)
	{
		bool bRet = regex_match(ele, match, reg);
		if (bRet)
		{
			std::string strMatchFile = strFilePath + ele;
			vMatchFile.push_back(strMatchFile);
		}
	}

	return nRet;
}

bool CBaseCmd::IsWildcard(std::string strPath)
{
	if (strPath.length() <= 0)
		return false;

	char drive[_MAX_DRIVE] = { 0 }, dir[_MAX_DIR] = { 0 }, fname[_MAX_FNAME] = { 0 }, ext[_MAX_EXT] = { 0 };
	_splitpath_s(strPath.c_str(), drive, dir, fname, ext);
	std::string strFileName = std::string(fname) + ext;
	std::string strFilePath = std::string(drive) + dir;
	//���·���м�û��*Ҳû��?,��ֱ�ӷ���
	size_t nPos1 = strFileName.find('*');
	size_t nPos2 = strFileName.find('?');
	if (nPos1 == std::string::npos && nPos2 == std::string::npos)
		return false;

	return true;
}

int CBaseCmd::GetChild(std::string strPath, std::vector<std::string>& vMatchFile, int nPathFormat)
{
	if (strPath.length() <= 0)
		return -1;

	int nRet = 0;

	if (nPathFormat == PATHFORMAT_REALPATH)
	{
		std::string strNewDir = strPath + "\\*.*";

		WIN32_FIND_DATA data;
		HANDLE hFind = FindFirstFile(strNewDir.c_str(), &data);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			throw std::exception("�����õ��ļ�Ŀ¼��");
		}

		do
		{
			if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
				continue;

			vMatchFile.push_back(data.cFileName);
		} while (FindNextFile(hFind, &data));

		FindClose(hFind);
	}
	else if (nPathFormat == PATHFORMAT_VIRTUALPATH)
	{
		CBase* pParent = GetPtrByPath(strPath);
		if (pParent && pParent->GetFileType() == FILETYPE_DIR)
		{
			for (auto ele : (dynamic_cast<CDir*>(pParent))->m_mapChild)
			{
				vMatchFile.push_back(ele.first);
			}
		}
		else
		{
			throw std::exception("ϵͳ�Ҳ���ָ��·����");
		}
	}
	return nRet;
}

void CDirCmd::PrintDefault(CBase* pBase)
{
	std::string strTime = pBase->GetTimeStr();
	std::cout << " " << pBase->GetAbsolutePath().c_str() << " ��Ŀ¼" << std::endl;

	char szBuf[1024] = { 0 };
	sprintf_s(szBuf, _countof(szBuf), "%s    <DIR>          .", strTime.c_str());
	std::cout << szBuf << std::endl;

	sprintf_s(szBuf, _countof(szBuf), "%s    <DIR>          ..", strTime.c_str());
	std::cout << szBuf << std::endl;

	m_nTotalDirCnt += 2;
}

void CDirCmd::PrintStatistics()
{
	if (m_nTotalFileCnt <= 0 && m_nTotalDirCnt <= 0)
	{
		//std::cout << std::endl << "�Ҳ����ļ�" << std::endl;
		return;
	}

	if (m_bS)
		std::cout << std::endl << "     �����ļ�����:" << std::endl;

	char szBuf[1024] = { 0 };
	sprintf_s(szBuf, _countof(szBuf), " %15lld ���ļ�%15lld �ֽ�"
		, m_nTotalFileCnt, m_nTotalFileSize);
	std::cout << szBuf << std::endl;

	sprintf_s(szBuf, _countof(szBuf), " %15lld ��Ŀ¼%15lld �����ֽ�"
		, m_nTotalDirCnt, m_nTotalSpaceSize - m_nTotalFileSize);
	std::cout << szBuf << std::endl;
}

void CDirCmd::PrintStatistics(CBase* pBase)
{
	int nSize = 0, nCnt = 0;
	if (!m_bAd)
	{
		switch (pBase->GetFileType())
		{
		case FILETYPE_DIR:
		{
			for (auto ele : dynamic_cast<CDir*>(pBase)->m_mapChild)
			{
				if (ele.second->GetFileType() == FILETYPE_FILE)
				{
					nSize += dynamic_cast<CFile*>(ele.second)->GetSize();
					nCnt++;
				}
				else if (ele.second->GetFileType() == FILETYPE_LINK)
				{
					nCnt++;
				}
			}
		}
			break;
		case FILETYPE_LINK:
			nCnt = 1;
			break;
		case FILETYPE_FILE:
		{
			nCnt = 1;
			nSize = dynamic_cast<CFile*>(pBase)->GetSize();
		}
			break;
		default:
			break;
		}

	}
	char szBuf[1024] = { 0 };
	sprintf_s(szBuf, _countof(szBuf), " %15d ���ļ�%15d �ֽ�", nCnt, nSize);
	std::cout << szBuf << std::endl;
}

int CDirCmd::Print(CBase* pBase)
{
	if (!pBase)
		return -1;

	if (pBase->GetFileType() == FILETYPE_FILE)
	{
		if (!m_bAd)
		{
			PrintFile(pBase);
		}

		if (m_bS)
		{
			PrintStatistics(pBase);
		}
	}
	else if(pBase->GetFileType() == FILETYPE_LINK)
	{
		if (!m_bAd)
		{
			PrintLink(pBase);
			PrintStatistics(pBase);
		}

		if (m_bS)
		{
			PrintStatistics(pBase);
		}
	}
	else
	{
		PrintDefault(pBase);
		CDir* pDir = (CDir*)pBase;
		for (auto ele : pDir->m_mapChild)
		{
			switch (ele.second->GetFileType())
			{
			case FILETYPE_UNKNOW:
				break;
			case FILETYPE_FILE:
				if (!m_bAd)
					PrintFile(ele.second);
				break;
			case FILETYPE_DIR:
				PrintDir(ele.second);
				break;
			case FILETYPE_LINK:
				if (!m_bAd)
					PrintLink(ele.second);
				break;
			default:
				break;
			}
		}

		if (m_bS)
			PrintStatistics(pDir);

		if (m_bS)
		{
			for (auto ele : pDir->m_mapChild)
			{
				if (ele.second->GetFileType() == FILETYPE_DIR)
				{
					Print(ele.second);
				}
			}
		}
	}

	return 0;
}

int CDirCmd::PrintDir(CBase* pBase) 
{
	if (!pBase)
		return -1;

	char szBuf[1024] = { 0 };
	sprintf_s(szBuf, _countof(szBuf), "%s    <DIR>          %s"
		, pBase->GetTimeStr().c_str(), pBase->GetName().c_str());
	std::cout << szBuf << std::endl;

	m_nTotalDirCnt++;
	return 0;
}

int CDirCmd::PrintFile(CBase* pBase)
{
	if (!pBase)
		return -1;

	CFile* pFile = (CFile*)pBase;
	std::string strTime = pFile->GetTimeStr();
	char szBuf[1024] = { 0 };
	sprintf_s(szBuf, _countof(szBuf), "%s%18d %s"
		, strTime.c_str(), pFile->GetSize(), pFile->GetName().c_str());
	std::cout << szBuf << std::endl;

	m_nTotalFileCnt++;
	m_nTotalFileSize += pFile->GetSize();
	return 0;
}

int CDirCmd::PrintLink(CBase* pBase)
{
	if (!pBase)
		return -1;

	CLink* pLink = (CLink*)pBase;
	std::string strTime = pLink->GetTimeStr();
	char szBuf[1024] = { 0 };
	sprintf_s(szBuf, _countof(szBuf), "%s    %s      %s [%s]"
		, strTime.c_str(), "<SYMLINK>", pLink->GetName().c_str(), pLink->GetLinkName().c_str());
	std::cout << szBuf << std::endl;
	m_nTotalFileCnt++;

	return 0;
}

int CDirCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() <= 0)
		{
			Print(m_pCur);
		}
		else
		{
			for (size_t i = 0; i < vParam.size(); i++)
			{
				CBase* pBase = GetPtrByPath(vParam[i]);
				if (pBase)
					Print(pBase);
				else
				{
					throw std::exception("�Ҳ����ļ���");
				}
			}
		}
	}
	catch (std::exception& e) 
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	PrintStatistics();
	return nRet;
}

int CDirCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 2)
		return -1;
	
	m_bAd = m_bS = false;

	int nRet = 0;
	for (auto ele : vOption)
	{
		if (0 == strcmp(ele.c_str(), "/ad") || 0 == strcmp(ele.c_str(), "/AD"))
		{
			m_bAd = true;
		}
		else if (0 == strcmp(ele.c_str(), "/s") || 0 == strcmp(ele.c_str(), "/S"))
		{
			m_bS = true;
		}
		else
		{
			nRet = -1;
			break;
		}
	}

	return nRet;
}

int CMdCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		for (auto ele : vParam)
		{
			std::string strAbsolutePath;
			int nRet = ToAbsolutePath(ele, strAbsolutePath);
			if (nRet < 0)
				continue;

			std::vector<std::string> vRoutePath;
			ParsePathRoute(strAbsolutePath, vRoutePath);

			CDir* pDir = dynamic_cast<CDir*>(GetRoot());
			//��·������Ҫ����i��1��ʼ
			for (size_t i = 1; i < vRoutePath.size(); i++)
			{
				if (vRoutePath[i] == ".")
					continue;

				if (vRoutePath[i] == "..")
				{
					pDir = dynamic_cast<CDir*>(pDir->m_pParent);
					continue;
				}

				auto it = pDir->m_mapChild.find(vRoutePath[i]);
				if (it != pDir->m_mapChild.end())
				{
					if (it->second->GetFileType() != FILETYPE_DIR)
					{
						char szError[512] = { 0 };
						sprintf_s(szError, _countof(szError), "��Ŀ¼���ļ� %s �Ѿ����ڡ�", it->first.c_str());
						throw std::exception(szError);
					}
					pDir = dynamic_cast<CDir*>(it->second);

					//���һ��Ŀ¼�����ڣ�����
					if (i == vRoutePath.size() - 1)
					{
						char szError[512] = { 0 };
						sprintf_s(szError, _countof(szError), "��Ŀ¼���ļ� %s �Ѿ����ڡ�", it->first.c_str());
						throw std::exception(szError);
					}
				}
				else
				{
					CBase* dir = new CDir(pDir, vRoutePath[i], time(0));
					pDir->m_mapChild[vRoutePath[i]] = dir;
					pDir = dynamic_cast<CDir*>(dir);
				}
			}
		}
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CMdCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 0)
		return -1;

	return 0;
}

int CCdCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() > 1)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		//���cd�޲���,��ֻ������ǰ����·������
		if (0 == vParam.size())
		{
			std::cout << m_pCur->GetAbsolutePath().c_str() << std::endl;
			return 0;
		}

		CBase* pCurBase = GetPtrByPath(vParam[0]);
		if (!pCurBase)
		{
			throw std::exception("ϵͳ�Ҳ���ָ����·����");
		}

		int nType = pCurBase->GetFileType();
		switch (nType)
		{
		case FILETYPE_UNKNOW:
		case FILETYPE_FILE:
			throw std::exception("Ŀ¼������Ч��");
		case FILETYPE_LINK:
		{
			while (pCurBase->GetFileType() == FILETYPE_LINK)
			{
				std::string strAbsLinkPath = dynamic_cast<CLink*>(pCurBase)->GetAbsoluteLinkPath();
				pCurBase = GetPtrByPath(strAbsLinkPath);
				if (!pCurBase)
				{
					throw std::exception("Ŀ¼������Ч��");
				}
			}

			if (pCurBase->GetFileType() != FILETYPE_DIR)
			{
				throw std::exception("Ŀ¼������Ч��");
			}
		}
		case FILETYPE_DIR:
		{
			m_pCur = pCurBase;
			*ppBase = m_pCur;
		}
		break;
		default:
			break;
		}
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CCdCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 0)
		return -1;

	return 0;
}

//��ʱ֧�ִ���ʵ�ļ�ϵͳ�������ļ�ϵͳ�����������ļ�ϵͳ���Կ���������copy @C:/dir E:/dir; copy @C:/dir/1.txt E:/dir1/2.txt
int CCopyCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;
	int nRet = 0;

	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 2)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		std::string strSrc = vParam[0];
		std::string strDes = vParam[1];
		int nDesPathType = PathFormat(strDes);
		if (nDesPathType != PATHFORMAT_VIRTUALPATH)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		int nSrcPathType = PathFormat(strSrc);
		if (nSrcPathType == PATHFORMAT_REALPATH)
		{
			//�ж��Ƿ���ͨ���
			m_bWildcard = IsWildcard(strSrc);

			//֧��ͨ���
			std::vector<std::string> vMatchFile;
			int nRet = GetMatchFile(strSrc, vMatchFile, PATHFORMAT_REALPATH);
			if (nRet != 0)
				return -1;

			for (auto ele : vMatchFile)
			{
				CopyReal2Virtual(ele, strDes);
			}
		}
		else if (nSrcPathType == PATHFORMAT_VIRTUALPATH)
		{
			std::string strAbsDes;
			ToAbsolutePath(strDes, strAbsDes);

			std::string strAbsSrc;
			ToAbsolutePath(strSrc, strAbsSrc);

			//�ж��Ƿ���ͨ���
			m_bWildcard = IsWildcard(strAbsSrc);

			if (m_bWildcard)
			{
				//֧��ͨ���
				std::vector<std::string> vMatchFile;
				int nRet = GetMatchFile(strAbsSrc, vMatchFile, PATHFORMAT_VIRTUALPATH);
				if (nRet != 0)
					return -1;

				for (auto ele : vMatchFile)
				{
					CopyVirtual2Virtual(ele, strAbsDes);
				}
			}
			else
			{
				CopyVirtual2Virtual(strAbsSrc, strAbsDes);
			}

		}
		else
		{
			throw std::exception("�����﷨����ȷ��");
		}
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CCopyCmd::CopyVirtual2Virtual(std::string strSrc, std::string strDes)
{
	CBase* pSrcBase = nullptr;
	if (0 != FileIsExist(strSrc, PATHFORMAT_VIRTUALPATH, &pSrcBase))
	{
		throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
	}

	CBase* pDesBase = nullptr;
	if (0 == FileIsExist(strDes, PATHFORMAT_VIRTUALPATH, &pDesBase))
	{
		if (pDesBase->GetFileType() == FILETYPE_DIR)
		{
			if (pSrcBase->GetFileType() == FILETYPE_DIR)
			{
				CopyVirtualDir2VirtualDir(strSrc, strDes);
			}
			else if (pSrcBase->GetFileType() == FILETYPE_FILE)
			{
				CopyVirtualFile2VirtualDir(strSrc, strDes);
			}
			else if (pSrcBase->GetFileType() == FILETYPE_LINK)
			{
				CopyVirtualLink2VirtualDir(strSrc, strDes);
			}
			else
			{
				throw std::exception("��֧�ֵ��ļ����͡�");
			}
		}
		else if(pDesBase->GetFileType() == FILETYPE_FILE || pDesBase->GetFileType() == FILETYPE_LINK)
		{
			if (pSrcBase->GetFileType() == FILETYPE_DIR)
			{
				//dir 2 file
				throw std::exception("Ŀ¼�޷����Ƹ��ļ���");
			}
			else if (pSrcBase->GetFileType() == FILETYPE_FILE)
			{
				CopyVirtualFile2VirtualFile(strSrc, strDes);
			}
			else if (pSrcBase->GetFileType() == FILETYPE_LINK)
			{
				CopyVirtualLink2VirtualFile(strSrc, strDes);
			}
			else
			{
				throw std::exception("��֧�ֵ��ļ����͡�");
			}
		}
		else
		{
			throw std::exception("��֧�ֵ��ļ����͡�");
		}
	}
	else
	{
		char drive[_MAX_DRIVE] = { 0 }, dir[_MAX_DIR], fname[_MAX_FNAME] = { 0 }, ext[_MAX_EXT] = { 0 };
		_splitpath_s(strDes.c_str(), drive, dir, fname, ext);
		std::string strDesPath = std::string(drive) + dir;
		std::string strDesName = std::string(fname) + ext;
		if (strDesName.length() <= 0)
		{
			throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
		}
		else
		{
			if (pSrcBase->GetFileType() == FILETYPE_DIR)
			{
				//dir 2 file
				throw std::exception("Ŀ¼�޷����Ƹ��ļ���");
			}
			else if (pSrcBase->GetFileType() == FILETYPE_FILE)
			{
				CopyVirtualFile2VirtualFile(strSrc, strDes);
			}
			else if (pSrcBase->GetFileType() == FILETYPE_LINK)
			{
				CopyVirtualLink2VirtualFile(strSrc, strDes);
			}
			else
			{
				throw std::exception("��֧�ֵ��ļ����͡�");
			}
		}
	}
	
	return 0;
}

int CCopyCmd::CopyVirtualFile2VirtualFile(std::string strSrc, std::string strDes)
{
	CBase* pSrcBase = GetPtrByPath(strSrc);
	if (!pSrcBase || pSrcBase->GetFileType() != FILETYPE_FILE)
	{
		throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
	}
	CFile* pSrcFile = dynamic_cast<CFile*>(pSrcBase);

	CBase* pDesBase = GetPtrByPath(strDes);
	if (pDesBase)
	{
		if (pDesBase->GetFileType() == FILETYPE_DIR)
		{
			throw std::exception("����ͬ��Ŀ¼��");
		}
		else
		{
			//����
			std::string strName = pDesBase->GetName();
			CBase* pParent = pDesBase->m_pParent;
			delete pDesBase, pDesBase = nullptr;
			pDesBase = new CFile(pParent, strName, time(0), pSrcFile->GetSize(), pSrcFile->GetData());
			dynamic_cast<CDir*>(pParent)->m_mapChild[strName] = pDesBase;
		}
	}
	else
	{
		char drive[_MAX_DRIVE] = { 0 }, dir[_MAX_DIR] = { 0 }, fname[_MAX_FNAME] = { 0 }, ext[_MAX_EXT] = { 0 };
		_splitpath_s(strDes.c_str(), drive, dir, fname, ext);
		std::string strPath = std::string(drive) + dir;
		std::string strName = std::string(fname) + ext;
		CBase* pParent = GetPtrByPath(strPath);
		if (!pParent)
		{
			throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
		}
		pDesBase = new CFile(pParent, strName, time(0), pSrcFile->GetSize(), pSrcFile->GetData());
		dynamic_cast<CDir*>(pParent)->m_mapChild[strName] = pDesBase;
	}

	return 0;
}

int CCopyCmd::CopyVirtualFile2VirtualDir(std::string strSrc, std::string strDes)
{
	CBase* pSrcBase = GetPtrByPath(strSrc);
	if (!pSrcBase || pSrcBase->GetFileType() != FILETYPE_FILE)
	{
		throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
	}

	CBase* pDesBase = GetPtrByPath(strDes);
	if (!pDesBase || pDesBase->GetFileType() != FILETYPE_DIR)
	{
		throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
	}

	CFile* pSrcFile = dynamic_cast<CFile*>(pSrcBase);
	CDir* pDesDir = dynamic_cast<CDir*>(pDesBase);

	std::string strName = pSrcFile->GetName();
	auto itFind = pDesDir->m_mapChild.find(strName);
	if (itFind != pDesDir->m_mapChild.end())
	{
		if (itFind->second->GetFileType() == FILETYPE_DIR)
		{
			throw std::exception("����ͬ��Ŀ¼��");
		}
		else
		{
			delete itFind->second, itFind->second = nullptr;
			CFile* pNewDes = new CFile(pDesDir, strName, time(0), pSrcFile->GetSize(), pSrcFile->GetData());
			pDesDir->m_mapChild[strName] = pNewDes;
		}
	}
	else
	{
		CFile* pNewDes = new CFile(pDesDir, strName, time(0), pSrcFile->GetSize(), pSrcFile->GetData());
		pDesDir->m_mapChild[strName] = pNewDes;
	}

	return 0;
}

int CCopyCmd::CopyVirtualLink2VirtualFile(std::string strSrc, std::string strDes)
{
	CBase* pBase = GetPtrByPath(strSrc);
	if (!pBase || pBase->GetFileType() != FILETYPE_LINK)
	{
		throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
	}

	CLink* pLink = dynamic_cast<CLink*>(pBase);
	
	//CopyVirtualFile2VirtualDir(pLink->GetLinkName(), strDes);
	CopyVirtual2Virtual(pLink->GetLinkName(), strDes);
	return 0;
}

int CCopyCmd::CopyVirtualLink2VirtualDir(std::string strSrc, std::string strDes)
{
	CBase* pBase = GetPtrByPath(strSrc);
	if (!pBase || pBase->GetFileType() != FILETYPE_LINK)
	{
		throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
	}

	CLink* pLink = dynamic_cast<CLink*>(pBase);
	std::string strAbsLinkPath = pLink->GetAbsoluteLinkPath();
	CopyVirtual2Virtual(strAbsLinkPath, strDes);
	return 0;
}

int CCopyCmd::CopyVirtualDir2VirtualDir(std::string strSrc, std::string strDes)
{
	CBase* pSrcBase = GetPtrByPath(strSrc);
	if (!pSrcBase || pSrcBase->GetFileType() != FILETYPE_DIR)
	{
		throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
	}

	CBase* pDesBase = GetPtrByPath(strSrc);
	if (!pDesBase || pDesBase->GetFileType() != FILETYPE_DIR)
	{
		throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
	}

	CDir* pDir = dynamic_cast<CDir*>(pSrcBase);
	for (auto ele : pDir->m_mapChild)
	{
		if (ele.second->GetFileType() == FILETYPE_FILE)
		{
			CopyVirtualFile2VirtualDir(ele.second->GetAbsolutePath(), strDes);
		}
		else if(ele.second->GetFileType() == FILETYPE_LINK)
		{
			CopyVirtualLink2VirtualDir(ele.second->GetAbsolutePath(), strDes);
		}
		else
			continue;
	}

	return 0;
}

int CCopyCmd::CopyReal2Virtual(std::string strSrc, std::string strDes)
{
	if (0 != FileIsExist(strSrc, PATHFORMAT_REALPATH, nullptr))
	{
		throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
	}

	//����ڵ���
	std::string strVirtualName;
	CBase* pBase = GetPtrByPath(strDes);
	if (!pBase)
	{
		//����ļ��Ҳ��������������ϼ�Ŀ¼
		std::string strNewDes = m_pCur->GetAbsolutePath() + "//" + strDes;
		char drive[_MAX_DRIVE] = { 0 };
		char dir[_MAX_DIR] = { 0 };
		char fname[_MAX_FNAME] = { 0 };
		char ext[_MAX_EXT] = { 0 };

		_splitpath_s(strNewDes.c_str()
			, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		std::string strPath = std::string(drive) + dir;
		pBase = GetPtrByPath(strPath);
		if (!pBase)
		{
			throw std::exception("ϵͳ�Ҳ���ָ����Ŀ¼��");
		}
		strVirtualName = std::string(fname) + ext;
	}

	int nDir = FileIsDir(strSrc);
	if (0 == nDir)
	{
		//Ŀ¼ֻ�ܸ��Ƹ�Ŀ¼
		if (pBase->GetFileType() == FILETYPE_DIR)
		{
			if (!m_bWildcard)
			{
				CopyRealDir2VirtualDir(strSrc, dynamic_cast<CDir*>(pBase));
			}
			else
			{
				std::vector<std::string> vRoute;
				ParsePathRoute(strSrc, vRoute);
				std::string strParent = vRoute[0];
				size_t i = 1;
				for (; i < vRoute.size() - 1; i++)
				{
					strParent += vRoute[i] + "\\";
				}
				CDir* pParent = new CDir(pBase, vRoute[i], time(0));
				dynamic_cast<CDir*>(pBase)->m_mapChild[pParent->GetName()] = pParent;
				CopyRealDir2VirtualDir(strSrc, dynamic_cast<CDir*>(pParent));
			}
		}
		else
		{
			//Ŀ¼���ܸ��Ƹ��ļ�
			throw std::exception("Ŀ¼�޷����Ƹ��ļ���");
		}
	}
	else
	{
		//�ļ����Ը��Ƹ�Ŀ¼Ҳ���Ը��Ƹ��ļ������ǲ��ܸ��Ƹ������ļ�
		if (pBase->GetFileType() == FILETYPE_DIR)
		{
			CopyRealFile2VirtualDir(strSrc, dynamic_cast<CDir*>(pBase), strVirtualName);
		}
		else if (pBase->GetFileType() == FILETYPE_FILE)
		{
			CopyRealFile2VirtualFile(strSrc, dynamic_cast<CFile*>(pBase));
		}
		else
		{
			throw std::exception("�ļ��޷����Ƹ������ļ���");
		}
	}

	return 0;
}

int CCopyCmd::CopyRealFile2VirtualDir(std::string strSrc, CDir* pDesDir, std::string strName)
{
	if (strName.length() == 0)
	{
		char drive[_MAX_DRIVE] = { 0 };
		char dir[_MAX_DIR] = { 0 };
		char fname[_MAX_FNAME] = { 0 };
		char ext[_MAX_EXT] = { 0 };

		_splitpath_s(strSrc.c_str()
			, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		strName = std::string(fname) + ext;
	}

	auto it = pDesDir->m_mapChild.find(strName);
	if (it != pDesDir->m_mapChild.end())
	{
		if (it->second->GetFileType() == FILETYPE_FILE)
			CopyRealFile2VirtualFile(strSrc, dynamic_cast<CFile*>(it->second));
		else
		{
			throw std::exception("����ͬ��Ŀ¼��");
		}
	}
	else
	{
		CFile* pFile = new CFile(pDesDir, strName, time(0), 0, nullptr);
		pDesDir->m_mapChild[strName] = pFile;
		CopyRealFile2VirtualFile(strSrc, pFile);
	}

	return 0;
}

int CCopyCmd::CopyRealFile2VirtualFile(std::string strSrc, CFile* pDesFile)
{
	int nSize = GetFileSize(strSrc, PATHFORMAT_REALPATH, nullptr);
	if (nSize < 0)
	{
		//�޷�ʶ����ļ�
		throw std::exception("�޷�ʶ����ļ���");
	}
	char* pData = new char[nSize];
	if (!pData)
	{
		std::cout << "out of memory" << std::endl;
		return -1;
	}
	GetFileData(strSrc, PATHFORMAT_REALPATH, (void*)pData, nSize, nullptr);
	if (pDesFile->m_pData)
	{
		delete[] pDesFile->m_pData, pDesFile->m_pData = nullptr;
		pDesFile->m_nSize = 0;
	}
	pDesFile->SetData(pData, nSize);
	if (pData)
		delete[] pData, pData = nullptr;

	return 0;
}

int CCopyCmd::CopyRealDir2VirtualDir(std::string strSrc, CDir* pDesDir) 
{
	std::string strNewDir = strSrc + "\\*.*";

	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(strNewDir.c_str(), &data);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		throw std::exception("�޷�ʶ���Ŀ¼��");
	}

	do
	{
		if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
			continue;

		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			CDir* pDir = nullptr;
			auto it = pDesDir->m_mapChild.find(data.cFileName);
			if (it != pDesDir->m_mapChild.end())
			{
				if (pDesDir->GetFileType() == FILETYPE_DIR)
					pDir = dynamic_cast<CDir*>(it->second);
				else
				{
					delete it->second, it->second = nullptr;
					//pDesDir->m_mapChild.erase(it);
					it->second = new CDir(pDesDir, data.cFileName, time(0));
					pDir = dynamic_cast<CDir*>(it->second);
				}
			}
			else
			{
				pDir = new CDir(pDesDir, data.cFileName, time(0));
			}

			pDesDir->m_mapChild[data.cFileName] = pDir;

			std::string strPath = strSrc + "//" + data.cFileName;
			CopyRealDir2VirtualDir(strPath, pDir);
		}
		else
		{
			std::string strFile = strSrc + "//" + data.cFileName;
			int nSize = GetFileSize(strFile.c_str(), PATHFORMAT_REALPATH, nullptr);
			if (nSize >= 0)
			{
				char* pData = nullptr;
				try
				{
					pData = new char[nSize];
					if (!pData)
						throw "";
				}
				catch (...)
				{
					std::cerr << "out of memory" << std::endl;
					exit(0);
				}
				GetFileData(strFile, PATHFORMAT_REALPATH, (void*)pData, nSize, nullptr);
				CFile* pFile = new CFile(pDesDir, data.cFileName, time(0), nSize, pData);
				if (!pData)
				{
					std::cout << "out of memory." << std::endl;
					exit(0);
				}
				auto itFind = pDesDir->m_mapChild.find(data.cFileName);
				if (itFind != pDesDir->m_mapChild.end())
					delete itFind->second, itFind->second = nullptr;

				pDesDir->m_mapChild[data.cFileName] = pFile;
				if (pData)
					delete[] pData, pData = nullptr;
			}
			else
			{
				//ERROR FILE,Դ�ļ������ڵķ����ļ�
				throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
			}

		}
	} while (FindNextFile(hFind, &data));

	FindClose(hFind);
	return 0;
}

int CCopyCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 0)
		return -1;

	return 0;
}


int CDelCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() <= 0)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		//ʹ������ƥ��
		std::vector<std::string> vDelPath;
		for (auto ele : vParam)
		{
			//�ж��Ƿ���ͨ���
			bool bWildcard = IsWildcard(ele);
			if (bWildcard)
			{
				std::string strAbsolutePath;
				ToAbsolutePath(ele, strAbsolutePath);
				int nRet = GetMatchFile(strAbsolutePath, vDelPath, PATHFORMAT_VIRTUALPATH);
				if (nRet < 0)
					continue;
			}
			else
			{
				vDelPath.push_back(ele);
			}
		}

		for (size_t i = 0; i < vDelPath.size(); i++)
		{
			CBase* pBase = GetPtrByPath(vDelPath[i]);
			if (!pBase)
			{
				std::string strAbsolutePath = m_pCur->GetAbsolutePath();
				strAbsolutePath = strAbsolutePath + "\\" + vDelPath[i];
				std::cerr << "�Ҳ��� " << strAbsolutePath.c_str() << std::endl;
				continue;
			}
			Del(pBase);
		}
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CDelCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 1)
		return -1;

	m_bS = false;
	int nRet = 0;
	for (auto ele : vOption)
	{
		if (0 == strcmp(ele.c_str(), "/s") || 0 == strcmp(ele.c_str(), "/S"))
		{
			m_bS = true;
		}
		else
		{
			nRet = -1;
			break;
		}
	}

	return nRet;
}

int CDelCmd::Del(CBase* pBase)
{
	if (!pBase)
		return -1;

	switch (pBase->GetFileType())
	{
	case FILETYPE_UNKNOW:
	case FILETYPE_LINK:
	case FILETYPE_FILE:
	{
		CDir* m_pParent = dynamic_cast<CDir*>(pBase->m_pParent);
		auto it = m_pParent->m_mapChild.find(pBase->GetName());
		m_pParent->m_mapChild.erase(it);
		delete pBase, pBase = nullptr;
	}
		break;
	case FILETYPE_DIR:
	{
		if (m_bS)
		{
			CDir* pDir = dynamic_cast<CDir*>(pBase);
			for (auto it : pDir->m_mapChild)
			{
				delete it.second, it.second = nullptr;
			}
			pDir->m_mapChild.erase(pDir->m_mapChild.begin(), pDir->m_mapChild.end());
		}
		else
		{
			std::cerr << "��֧��ɾ��Ŀ¼�ļ���" << std::endl;
		}
	}
	break;
	default:
		break;
	}

	return 0;
}

int CRdCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() <= 0)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		for (size_t i = 0; i < vParam.size(); i++)
		{
			CBase* pBase = GetPtrByPath(vParam[i]);
			if (!pBase)
			{
				std::cerr << "ϵͳ�Ҳ���ָ�����ļ���" << std::endl;
				continue;
			}
			if (pBase == m_pCur && pBase != GetRoot())
			{
				std::cerr << "��һ����������ʹ�ô��ļ��������޷����ʡ�" << std::endl;
			}

			CDir* pDir = dynamic_cast<CDir*>(pBase);
			if (m_pCur == pDir)
			{
				for (auto& ele : pDir->m_mapChild)
				{
					delete ele.second, ele.second = nullptr;
				}
				pDir->m_mapChild.erase(pDir->m_mapChild.begin(), pDir->m_mapChild.end());
			}
			else
				Rd(pBase);
		}
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CRdCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 1)
		return -1;

	m_bS = false;
	int nRet = 0;
	for (auto ele : vOption)
	{
		if (0 == strcmp(ele.c_str(), "/s"))
		{
			m_bS = true;
		}
		else
		{
			nRet = -1;
			break;
		}
	}

	return nRet;
}

int CRdCmd::Rd(CBase* pBase)
{
	if (!pBase)
		return -1;

	int nRet = 0;
	switch (pBase->GetFileType())
	{
	case FILETYPE_UNKNOW:
	case FILETYPE_LINK:
	case FILETYPE_FILE:
	{
		throw std::exception("Ŀ¼������Ч��");
	}
	break;
	case FILETYPE_DIR:
	{
		if (m_bS)
		{
			CDir* pParent = dynamic_cast<CDir*>(pBase->m_pParent);
			auto it = pParent->m_mapChild.find(pBase->GetName());
			if (it != pParent->m_mapChild.end())
			{
				delete it->second, it->second = nullptr;
				pParent->m_mapChild.erase(it);
			}
		}
		else
		{
			CDir* pDir = dynamic_cast<CDir*>(pBase);
			if (pDir->m_mapChild.size() > 0)
			{
				throw std::exception("Ŀ¼���ǿա�");
			}
			else
			{
				CDir* pParent = dynamic_cast<CDir*>(pDir->m_pParent);
				auto it = pParent->m_mapChild.find(pDir->GetName());
				if (it != pParent->m_mapChild.end())
				{
					delete it->second, it->second = nullptr;
					pParent->m_mapChild.erase(it);
				}
			}
		}
	}
	break;
	default:
		break;
	}

	return 0;
}

//rd src dst��dstֻ��Ϊ�ļ��������ܹ���·��
int CRenCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 2)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (strchr(vParam[1].c_str(), '\\') || strchr(vParam[1].c_str(), '/'))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		CBase* pSrcBase = GetPtrByPath(vParam[0]);
		if (!pSrcBase)
		{
			throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
		}

		CBase* pDesBase = GetPtrByPath(vParam[1]);
		if (pDesBase)
		{
			throw std::exception("����һ�������ļ��������Ҳ����ļ���");
		}

		Ren(pSrcBase, vParam[1]);
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CRenCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() != 0)
		return -1;

	return 0;
}

int CRenCmd::Ren(CBase* pBase, std::string strName)
{
	if (!pBase || strName.length() <= 0)
		return -1;

	int nRet = -1;
	CDir* pParent = dynamic_cast<CDir*>(pBase->m_pParent);
	if (pParent)
	{
		auto it = pParent->m_mapChild.find(pBase->GetName());
		if (it != pParent->m_mapChild.end())
		{
			pParent->m_mapChild.erase(it);
			pParent->m_mapChild[strName] = pBase;
			pBase->SetName(strName);
			nRet = 0;
		}
	}

	return nRet;
}

int CMoveCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 2)
		{
			throw std::exception("�����﷨����ȷ��");
		}
		std::string strSrc = vParam[0];
		std::string strDes = vParam[1];

		CBase* pSrcBase = GetPtrByPath(strSrc);
		if (!pSrcBase)
		{
			throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
		}

		Move(pSrcBase, strDes);
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CMoveCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() > 1)
		return -1;

	m_bY = false;
	int nRet = 0;
	for (auto ele : vOption)
	{
		if (0 == strcmp(ele.c_str(), "/y") || 0 == strcmp(ele.c_str(), "/Y"))
		{
			m_bY = true;
		}
		else
		{
			nRet = -1;
			break;
		}
	}

	return nRet;
}

/************************************************************************/
/* Ŀ¼���ļ������ļ����ڣ���ɾ���ļ�����Ŀ¼�Ƶ�des�����ļ������ڣ��������Ŀ¼����Ŀ¼�ƶ�des */
/* Ŀ¼��Ŀ¼����Ŀ¼������������Ŀ¼����Ŀ¼������������������Ŀ¼                        */
/* �ļ����ļ����ļ����ڼ����ǣ��ļ����������Ƶ�Ŀ��Ŀ¼                                */
/************************************************************************/
int CMoveCmd::Move(CBase* pSrcBase, std::string strDes) 
{
	CBase* pDesBase = GetPtrByPath(strDes);
	if (pDesBase)
	{
		if (pDesBase->GetFileType() == FILETYPE_DIR)
		{
			auto it = dynamic_cast<CDir*>(pDesBase)->m_mapChild.find(pSrcBase->GetName());
			if (it != dynamic_cast<CDir*>(pDesBase)->m_mapChild.end())
			{
				if (!m_bY)
				{
					throw std::exception("�ļ��Ѿ����ڡ�");
				}
				delete it->second, it->second = nullptr;
				(dynamic_cast<CDir*>(pDesBase))->m_mapChild.erase(it);
				(dynamic_cast<CDir*>(pDesBase))->m_mapChild.insert(std::make_pair(pSrcBase->GetName(), pSrcBase));
				pSrcBase->m_pParent = pDesBase;
			}
			else
			{
				CDir* pSrcParentBase = dynamic_cast<CDir*>(pSrcBase->m_pParent);
				auto it = pSrcParentBase->m_mapChild.find(pSrcBase->GetName());
				if (it != pSrcParentBase->m_mapChild.end())
				{
					pSrcParentBase->m_mapChild.erase(it);
					dynamic_cast<CDir*>(pDesBase)->m_mapChild.insert(std::make_pair(pSrcBase->GetName(), pSrcBase));
					pSrcBase->m_pParent = pDesBase;
				}
			}
		}
		else
		{
			if (!m_bY)
			{
				throw std::exception("�ļ��Ѿ����ڡ�");
			}
			//������Ǹ��ļ��У���ô������ļ�ɾ������Src��ӽ�������
			CDir* pDesParentBase = dynamic_cast<CDir*>(pDesBase->m_pParent);
			CDir* pSrcParentBase = dynamic_cast<CDir*>(pSrcBase->m_pParent);
			auto itSrc = pSrcParentBase->m_mapChild.find(pSrcBase->GetName());
			if (itSrc != pSrcParentBase->m_mapChild.end())
			{
				pSrcParentBase->m_mapChild.erase(itSrc);
			}
			pSrcBase->m_pParent = pDesParentBase;

			auto itDes = pDesParentBase->m_mapChild.find(pDesBase->GetName());
			pSrcBase->SetName(pDesBase->GetName());

			if (itDes != pDesParentBase->m_mapChild.end())
			{
				delete itDes->second, itDes->second = nullptr;
				pDesParentBase->m_mapChild.erase(itDes);
			}

			pDesParentBase->m_mapChild.insert(std::make_pair(pSrcBase->GetName(), pSrcBase));
		}
	}
	else
	{
		//Ŀ�겻���ڣ���Դ��������������
		std::string strAbsolutePath;
		ToAbsolutePath(strDes, strAbsolutePath);
		size_t nPos = -1;
		size_t nPos1 = strAbsolutePath.find_last_of('\\');
		if (nPos1 != std::string::npos)
			nPos = nPos1;
		size_t nPos2 = strAbsolutePath.find_last_of('/');
		if (nPos2 != std::string::npos && nPos2 > nPos1)
			nPos = nPos2;
		if (nPos < 0)
		{
			char szError[1024] = { 0 };
			sprintf_s(szError, _countof(szError), "ϵͳ�Ҳ���ָ����·����\n�ƶ���         0 ���ļ���\n");
			throw std::exception(szError);
		}
		std::string strDesPath = strAbsolutePath;
		strDesPath.erase(nPos, strDesPath.length() - nPos);
		std::string strDesName = strAbsolutePath;
		strDesName.erase(0, nPos + 1);

		CBase* pDesParentBase = GetPtrByPath(strDesPath);
		if (!pDesParentBase || pDesParentBase->GetFileType() != FILETYPE_DIR)
		{
			char szError[1024] = { 0 };
			sprintf_s(szError, _countof(szError), "��������\n�ƶ���         0 ���ļ���\n");
			throw std::exception(szError);
		}

		CDir* pSrcParentBase = dynamic_cast<CDir*>(pSrcBase->m_pParent);
		(dynamic_cast<CDir*>(pDesParentBase))->m_mapChild.insert(std::make_pair(strDesName, pSrcBase));
		auto it = pSrcParentBase->m_mapChild.find(pSrcBase->GetName());
		if (it != pSrcParentBase->m_mapChild.end())
		{
			pSrcParentBase->m_mapChild.erase(it);
		}
		pSrcBase->SetName(strDesName);
		pSrcBase->m_pParent = pDesParentBase;
	}

	return 0;
}

//mklink link src(windows)
int CMklinkCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 2)
		{
			throw std::exception("�����﷨����ȷ��");
		}
		std::string strLink = vParam[0];
		std::string strSrc = vParam[1];

		CBase* pSrc = GetPtrByPath(strSrc);
		if (!pSrc)
		{
			throw std::exception("ϵͳ�Ҳ���ָ����·����");
		}

		CBase* pLink = GetPtrByPath(strLink);
		if (pLink)
		{
			throw std::exception("���ļ��Ѵ���ʱ���޷��������ļ���");
		}

		std::string strAbsolutePath;
		ToAbsolutePath(strLink, strAbsolutePath);
		char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_splitpath_s(strAbsolutePath.c_str(), drive, dir, fname, ext);
		std::string strLinkPath = std::string(drive) + dir;
		std::string strLinkName = std::string(fname) + ext;

		CBase* pLinkParent = GetPtrByPath(strLinkPath);
		if (!pLinkParent || pLinkParent->GetFileType() != FILETYPE_DIR)
		{
			throw std::exception("ϵͳ�Ҳ���ָ����·����");
		}

		pLink = new CLink(pLinkParent, strLinkName, time(0));
		dynamic_cast<CLink*>(pLink)->SetLinkName(strSrc);
		(dynamic_cast<CDir*>(pLinkParent))->m_mapChild.insert(std::make_pair(strLinkName, pLink));
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CMklinkCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() != 0)
		return -1;

	return 0;
}

int CSaveCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 1)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		std::string strDesPath = vParam[0];
		if (PathFormat(strDesPath) != PATHFORMAT_REALPATH)
		{
			throw std::exception("·����ʽ����ȷ��");
		}

		CBase* pRoot = GetRoot();
		std::ofstream os(strDesPath, std::ios::binary);
		m_pSerialize->Serialize(os, dynamic_cast<CDir*>(pRoot));
		os.close();

	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CSaveCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() != 0)
		return -1;

	return 0;
}

int CLoadCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;

	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 1)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		std::string strDesPath = vParam[0];
		if (PathFormat(strDesPath) != PATHFORMAT_REALPATH)
		{
			throw std::exception("·����ʽ����ȷ��");
		}

		if (FileIsExist(strDesPath, PATHFORMAT_REALPATH, nullptr) != 0)
		{
			throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
		}

		CDir* pRoot = dynamic_cast<CDir*>(GetRoot());
		for (auto& ele : pRoot->m_mapChild)
		{
			if (ele.second)
			{
				delete ele.second, ele.second = nullptr;
			}
		}
		pRoot->m_mapChild.erase(pRoot->m_mapChild.begin(), pRoot->m_mapChild.end());

		std::ifstream is(strDesPath, std::ios::binary);
		m_pSerialize->DeSerialize(is, pRoot);
		is.close();
		pRoot->m_pParent = pRoot;
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}
	
	return nRet;
}

int CLoadCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() != 0)
		return -1;

	return 0;
}

int CClsCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	system("cls");
	return 0;
}

int CClsCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() != 0)
		return -1;

	return 0;
}


int CMoreCmd::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pCur)
		return -1;
	
	int nRet = 0;
	try
	{
		if (0 != ParseOptions(vOption))
		{
			throw std::exception("�����﷨����ȷ��");
		}

		if (vParam.size() != 1)
		{
			throw std::exception("�����﷨����ȷ��");
		}

		CBase* pBase = GetPtrByPath(vParam[0]);
		if (!pBase)
		{
			throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
		}
		else
		{
			if (pBase->GetFileType() == FILETYPE_FILE)
			{
				int size = dynamic_cast<CFile*>(pBase)->GetSize();
				char* data = dynamic_cast<CFile*>(pBase)->GetData();
				std::string strBuf(data, size);
				std::cout << "size : " << size << std::endl;
				std::cout << strBuf.c_str() << std::endl;
			}
			else if (pBase->GetFileType() == FILETYPE_LINK)
			{
				while (1)
				{
					CLink* pLink = dynamic_cast<CLink*>(pBase);
					std::string strAbsLinkPath = pLink->GetAbsoluteLinkPath();
					pBase = GetPtrByPath(strAbsLinkPath);
					if (!pBase)
					{
						throw std::exception("ϵͳ�Ҳ���ָ�����ļ���");
					}
					if (pBase->GetFileType() == FILETYPE_LINK)
						continue;
					else if (pBase->GetFileType() == FILETYPE_DIR)
					{
						throw std::exception("ֻ���ļ�֧�ִ�ָ�");
					}
					else
					{
						std::cout << "size : " << dynamic_cast<CFile*>(pBase)->GetSize() << std::endl;
						std::cout << dynamic_cast<CFile*>(pBase)->GetData() << std::endl;
						break;
					}
				}
			}
			else
			{
				throw std::exception("ֻ���ļ�֧�ִ�ָ�");
			}
		}
	}
	catch (std::exception& e)
	{
		nRet = -1;
		std::cerr << e.what() << std::endl;
	}
	catch (...)
	{
		nRet = -1;
		std::cerr << "unknow error." << std::endl;
	}

	return nRet;
}

int CMoreCmd::ParseOptions(std::vector<std::string>& vOption)
{
	if (vOption.size() != 0)
		return -1;

	return 0;
}
