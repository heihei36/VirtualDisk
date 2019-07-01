#ifndef __CCOMMAND_H__
#define __CCOMMAND_H__

#include "CFileStat.h"
#include "CSerialize.h"
#include <vector>

enum enCmdType
{
	CMDTYPE_UNKNOW = 100,
	CMDTYPE_DIR,
	CMDTYPE_MD,
	CMDTYPE_CD,
	CMDTYPE_COPY,
	CMDTYPE_DEL,
	CMDTYPE_RD,
	CMDTYPE_REN,
	CMDTYPE_MOVE,
	CMDTYPE_MKLINK,
	CMDTYPE_SAVE,
	CMDTYPE_LOAD,
	CMDTYPE_CLS,
	CMDTYPE_MORE
};

enum enPathType
{
	PATHTYPE_ERROR = -1,
	PATHTYPE_ABSOLUTE,
	PATHTYPE_RELATIVE
};

enum enPathFormat
{
	PATHFORMAT_ERROR = -2,
	PATHFORMAT_EMPTY = -1,
	PATHFORMAT_VIRTUALPATH = 0,
	PATHFORMAT_REALPATH = 1
};

class CBaseCmd 
{
public:
	CBaseCmd(CBase* pBase) 
	{
		m_pCur = pBase;
	}

	virtual ~CBaseCmd() 
	{
	
	}

public:
	//CBase*参数只对cd命令有效
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr) = 0;
	virtual int ParseOptions(std::vector<std::string>& vOption) = 0;
	
protected:
	CBase* GetRoot();
	CBase* GetPtrByPath(std::string strPath);

	bool IsWildcard(std::string strPath);
	
	int FileIsDir(std::string strPath);
	int PathFormat(std::string& strPath);
	int PathType(std::string strPath);//false相对路径；true绝对路径
	int ToAbsolutePath(std::string strPath, std::string& strAbsolutePath);
	int ParsePathRoute(std::string strPath, std::vector<std::string>& vPathRoute);
	
	int FileIsExist(std::string strPath, int nFileType, CBase** ppBase);
	int GetFileSize(std::string strPath, int nFileType, CBase** ppBase);
	int GetFileData(std::string strPath, int nFileType, void* pData, int& nSize, CBase** ppBase);
	int GetMatchFile(std::string strPath, std::vector<std::string>& vMatchFile, int nPathFormat);
	int GetChild(std::string strPath, std::vector<std::string>& vMatchFile, int nPathFormat);

public:
	CBase* m_pCur;

protected:
	std::vector<std::string> m_vOption;
};

class CDirCmd : public CBaseCmd 
{
public:
	CDirCmd(CBase* pBase)
		: CBaseCmd(pBase)
		, m_bAd(false)
		, m_bS(false)
		, m_nTotalDirCnt(0)
		, m_nTotalFileCnt(0)
		, m_nTotalFileSize(0)
		, m_nTotalSpaceSize((long long)MAXMEMORYSIZE)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	//只输出子目录
	bool m_bAd;
	//输出目录及子目录中所有的文件
	bool m_bS;

private:
	long long m_nTotalDirCnt;
	long long m_nTotalFileCnt;
	long long m_nTotalFileSize;
	const long long m_nTotalSpaceSize;

private:
	int Print(CBase* pBase);
	int PrintDir(CBase* pBase);
	int PrintFile(CBase* pBase);
	int PrintLink(CBase* pBase);
	void PrintDefault(CBase* pBase);

	void PrintStatistics();
	void PrintStatistics(CBase* pBase);
};

class CMdCmd : public CBaseCmd 
{
public:
	CMdCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);
};

class CCdCmd : public CBaseCmd
{
public:
	CCdCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);
};

class CCopyCmd : public CBaseCmd 
{
public:
	CCopyCmd(CBase* pBase)
		: CBaseCmd(pBase)
		, m_bWildcard(false)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	int CopyReal2Virtual(std::string strSrc, std::string strDes);
	int CopyVirtual2Virtual(std::string strSrc, std::string strDes);
 	int CopyRealDir2VirtualDir(std::string strSrc, CDir* pDesDir);
	int CopyRealFile2VirtualDir(std::string strSrc, CDir* pDesDir, std::string strName);
	int CopyRealFile2VirtualFile(std::string strSrc, CFile* pDesFile);

	int CopyVirtualFile2VirtualFile(std::string strSrc, std::string strDes);
	int CopyVirtualFile2VirtualDir(std::string strSrc, std::string strDes);
	int CopyVirtualLink2VirtualFile(std::string strSrc, std::string strDes);
	int CopyVirtualLink2VirtualDir(std::string strSrc, std::string strDes);
	int CopyVirtualDir2VirtualDir(std::string strSrc, std::string strDes);

private:
	//是否含有通配符
	bool m_bWildcard;
};

class CDelCmd : public CBaseCmd 
{
public:
	CDelCmd(CBase* pBase)
		: CBaseCmd(pBase)
		, m_bS(false)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	int Del(CBase* pBase);

private:
	//递归删除目录及其所有子目录下指定文件
	bool m_bS;
};

class CRdCmd : public CBaseCmd
{
public:
	CRdCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	int Rd(CBase* pBase);

private:
	//递归删除目录及其所有子目录下指定文件
	bool m_bS;
};

class CRenCmd : public CBaseCmd
{
public:
	CRenCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	int Ren(CBase* pBase, std::string strName);
};

class CMoveCmd : public CBaseCmd
{
public:
	CMoveCmd(CBase* pBase)
		: CBaseCmd(pBase)
		, m_bY(false)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	int Move(CBase* pSrcBase, std::string strDes);

private:
	bool m_bY;
};

class CMklinkCmd : public CBaseCmd
{
public:
	CMklinkCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);
};

class CSaveCmd : public CBaseCmd
{
public:
	CSaveCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	std::shared_ptr<CBaseSerialize> m_pSerialize;
};

class CLoadCmd : public CBaseCmd
{
public:
	CLoadCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);

private:
	std::shared_ptr<CBaseSerialize> m_pSerialize;
};

class CClsCmd : public CBaseCmd
{
public:
	CClsCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);
};

class CMoreCmd : public CBaseCmd
{
public:
	CMoreCmd(CBase* pBase)
		: CBaseCmd(pBase)
	{

	}

public:
	virtual int ExecCmd(std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
		, CBase** ppBase = nullptr);
	virtual int ParseOptions(std::vector<std::string>& vOption);
};

#endif

