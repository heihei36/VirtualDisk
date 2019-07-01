#include "stdafx.h"
#include "CCommandFactory.h"

CAbstractFactory::CAbstractFactory() 
{

}

CAbstractFactory::~CAbstractFactory()
{

}

CCmdFactory::CCmdFactory()
{
	m_pBase = nullptr;
}

CCmdFactory::~CCmdFactory()
{
	if (m_pBase)
		delete m_pBase, m_pBase = nullptr;
}

int CCmdFactory::CreateCmdInstance(CBase* pCur, std::string strCmdType, std::map<std::string, int>& mapCmdSet)
{
	if (m_pBase)
		delete m_pBase, m_pBase = nullptr;

	switch (mapCmdSet[strCmdType])
	{
	case CMDTYPE_DIR:
		m_pBase = new CDirCmd(pCur);
		break;
	case CMDTYPE_MD:
		m_pBase = new CMdCmd(pCur);
		break;
	case CMDTYPE_CD:
		m_pBase = new CCdCmd(pCur);
		break;
	case CMDTYPE_COPY:
		m_pBase = new CCopyCmd(pCur);
		break;
	case CMDTYPE_DEL:
		m_pBase = new CDelCmd(pCur);
		break;
	case CMDTYPE_RD:
		m_pBase = new CRdCmd(pCur);
		break;
	case CMDTYPE_REN:
		m_pBase = new CRenCmd(pCur);
		break;
	case CMDTYPE_MOVE:
		m_pBase = new CMoveCmd(pCur);
		break;
	case CMDTYPE_MKLINK:
		m_pBase = new CMklinkCmd(pCur);
		break;
	case CMDTYPE_SAVE:
		m_pBase = new CSaveCmd(pCur);
		break;
	case CMDTYPE_LOAD:
		m_pBase = new CLoadCmd(pCur);
		break;
	case CMDTYPE_CLS:
		m_pBase = new CClsCmd(pCur);
		break;
	case CMDTYPE_MORE:
		m_pBase = new CMoreCmd(pCur);
		break;
	case CMDTYPE_UNKNOW:
	default:
	{
		char szError[512] = { 0 };
		sprintf_s(szError, _countof(szError),
			"\'%s\' 不是内部或外部命令，也不是可运行的程序或批处理文件。", strCmdType.c_str());
		std::cerr << szError << std::endl;
		return -1;
	}
	}

	m_pBase->m_pCur = pCur;
	return 0;
}

int CCmdFactory::ExecCmd(std::vector<std::string>& vOption, std::vector<std::string>& vParam, CBase** ppBase)
{
	if (!m_pBase)
	{
		return -1;
	}

	return m_pBase->ExecCmd(vOption, vParam, ppBase);
}

