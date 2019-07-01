#ifndef __CCOMMANDFACTORY_H__
#define __CCOMMANDFACTORY_H__
#include "CCommand.h"

class CAbstractFactory 
{
public:
	CAbstractFactory();
	virtual ~CAbstractFactory();

public:
	virtual int CreateCmdInstance(CBase* pCur, std::string strCmdType, std::map<std::string, int>& mapCmdSet) = 0;
};

class CCmdFactory : public CAbstractFactory 
{
public:
	CCmdFactory();
	~CCmdFactory();

public:
	CBaseCmd* m_pBase;

public:
	virtual int CreateCmdInstance(CBase* pCur, std::string strCmdType, std::map<std::string, int>& mapCmdSet);
	int ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** pBase = nullptr);
};

#endif
