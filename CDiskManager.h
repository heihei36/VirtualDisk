#ifndef __CDISKMANAGER_H__
#define __CDISKMANAGER_H__

#include "CCommandFactory.h"
#include "CCmdParse.h"

//由于程序是单线程的，暂不考虑线程安全问题。
class CDiskManager 
{
private:
	CDiskManager();
	CDiskManager(const CDiskManager& manager);
	CDiskManager& operator=(CDiskManager& manager);

public:
	CBase* m_pRoot;
	CBase* m_pCur;
	std::map<std::string, int> m_mapCmdSet;
	static CDiskManager* m_pInstance;

public:
	~CDiskManager();

	void GenerateCmdSet();
	void Init(const char* pDrive);
	void DoWork();
	static CDiskManager* GetInstance();

private:
	std::string m_strCmd;
	std::vector<std::string> m_vCmdParam;
	std::shared_ptr<CCmdParse> m_pCmdParse;
};

#endif
