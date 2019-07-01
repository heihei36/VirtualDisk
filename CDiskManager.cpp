#include "stdafx.h"
#include "CDiskManager.h"
#include "CCommandFactory.h"

CDiskManager* CDiskManager::m_pInstance = nullptr;

CDiskManager::CDiskManager() 
{
	m_pRoot = nullptr;
	m_pCur = nullptr;
	m_pCmdParse = std::make_shared<CCmdParse>();
}

CDiskManager::CDiskManager(const CDiskManager& manager)
{

}

CDiskManager& CDiskManager::operator=(CDiskManager& manager)
{
	return *this;
}

CDiskManager::~CDiskManager() 
{

}

void CDiskManager::GenerateCmdSet()
{
	m_mapCmdSet["dir"] = CMDTYPE_DIR;
	m_mapCmdSet["DIR"] = CMDTYPE_DIR;

	m_mapCmdSet["md"] = CMDTYPE_MD;
	m_mapCmdSet["MD"] = CMDTYPE_MD;

	m_mapCmdSet["cd"] = CMDTYPE_CD;
	m_mapCmdSet["CD"] = CMDTYPE_CD;

	m_mapCmdSet["copy"] = CMDTYPE_COPY;
	m_mapCmdSet["COPY"] = CMDTYPE_COPY;

	m_mapCmdSet["del"] = CMDTYPE_DEL;
	m_mapCmdSet["DEL"] = CMDTYPE_DEL;

	m_mapCmdSet["rd"] = CMDTYPE_RD;
	m_mapCmdSet["RD"] = CMDTYPE_RD;

	m_mapCmdSet["ren"] = CMDTYPE_REN;
	m_mapCmdSet["REN"] = CMDTYPE_REN;

	m_mapCmdSet["move"] = CMDTYPE_MOVE;
	m_mapCmdSet["MOVE"] = CMDTYPE_MOVE;

	m_mapCmdSet["mklink"] = CMDTYPE_MKLINK;
	m_mapCmdSet["MKLINK"] = CMDTYPE_MKLINK;

	m_mapCmdSet["save"] = CMDTYPE_SAVE;
	m_mapCmdSet["SAVE"] = CMDTYPE_SAVE;

	m_mapCmdSet["load"] = CMDTYPE_LOAD;
	m_mapCmdSet["LOAD"] = CMDTYPE_LOAD;

	m_mapCmdSet["cls"] = CMDTYPE_CLS;
	m_mapCmdSet["CLS"] = CMDTYPE_CLS;

	m_mapCmdSet["more"] = CMDTYPE_MORE;
	m_mapCmdSet["MORE"] = CMDTYPE_MORE;
}

void CDiskManager::Init(const char* pDrive) 
{
	try
	{
		m_pRoot = new CDir(nullptr, pDrive, time(0));
		if (!m_pRoot)
			throw "";
	}
	catch (...)
	{
		std::cout << "out of memory" << std::endl;
		exit(0);
	}
	m_pRoot->m_pParent = m_pRoot;
	m_pCur = m_pRoot;
	m_pRoot->GetAbsolutePath();

	GenerateCmdSet();
}

void CDiskManager::DoWork() 
{
	while (1)
	{
		std::cout << m_pCur->GetAbsolutePath() << "\\>";

		char szCmd[1024] = { 0 };
		std::cin.getline(szCmd, sizeof szCmd);

		std::string strCmdType;
		std::vector<std::string> vOption;
		std::vector<std::string> vParam;
		m_pCmdParse->Parse(szCmd, strCmdType, vOption, vParam);
		if (strCmdType.length() > 0)
		{
			if (_stricmp(strCmdType.c_str(), "exit") == 0)
				break;

			std::shared_ptr<CCmdFactory> pCmd = std::make_shared<CCmdFactory>();
			pCmd->CreateCmdInstance(m_pCur, strCmdType, m_mapCmdSet);

			pCmd->ExecCmd(vOption, vParam, &m_pCur);

			std::cout << std::endl;
		}
	}
}

CDiskManager* CDiskManager::GetInstance() 
{
	if (m_pInstance == nullptr)
		m_pInstance = new CDiskManager();

	return m_pInstance;
}

