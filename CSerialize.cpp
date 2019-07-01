#include "stdafx.h"
#include "CSerialize.h"

#define DEBUGSERIALIZE 0

CBaseSerialize::CBaseSerialize()
{

}

CBaseSerialize::~CBaseSerialize()
{

}

//基本数据类型
template<typename T>
void CBaseSerialize::Serialize(std::ofstream& os, T& value)
{
#if DEBUGSERIALIZE
	std::cout << "line:" << __LINE__ << "  value:" << value << std::endl;
#endif
	os.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

//基本数据类型数组
template<typename VALUE, typename SIZE>
void CBaseSerialize::Serialize(std::ofstream&os, VALUE* value, SIZE size)
{
	if (!value || size <= 0)
		return;
#if DEBUGSERIALIZE
	std::cout << "line:" << __LINE__ << "  value:" << value << "  size:" << size << std::endl;
#endif
	size_t nSize = sizeof(VALUE) * size;
	os.write(reinterpret_cast<const char*>(value), sizeof(VALUE) * size);
}

//string
void CBaseSerialize::Serialize(std::ofstream& os, std::string value)
{
	//先序列化数据长度，再序列化数据内容
	size_t size = value.size();
	Serialize(os, size);
	Serialize(os, value.data(), value.size());
#if DEBUGSERIALIZE
	std::cout << "line:" << __LINE__ << "  value:" << value.data() << "  size:" << size << std::endl;
#endif
}

void CBaseSerialize::Serialize(std::ofstream& os, CBase* pBase)
{
	Serialize(os, pBase->GetAbsolutePath());

	Serialize(os, pBase->GetName());

	int type = pBase->GetFileType();
	Serialize(os, type);

	time_t time = pBase->GetTime();
	Serialize(os, time);
}

void CBaseSerialize::Serialize(std::ofstream& os, CFile* pFile)
{
	Serialize(os, (CBase*)pFile);
	int nSize = pFile->GetSize();
	Serialize(os, nSize);
	Serialize(os, pFile->GetData(), pFile->GetSize());
}

void CBaseSerialize::Serialize(std::ofstream& os, CDir* pDir)
{
	Serialize(os, (CBase*)pDir);
	Serialize(os, pDir->GetChild());
}

void CBaseSerialize::Serialize(std::ofstream& os, CLink* pLink)
{
	Serialize(os, (CBase*)pLink);
	Serialize(os, pLink->GetLinkName());
}

//map
void CBaseSerialize::Serialize(std::ofstream& os, std::map <std::string, CBase*>& child)
{
	size_t size = child.size();
	Serialize(os, size);

	for (auto ele : child)
	{
		Serialize(os, ele.first);
		int type = FILETYPE_UNKNOW;
		if (ele.second)
		{
			type = ele.second->GetFileType();
		}
		Serialize(os, type);

		CBase* pBase = ele.second;
		if (pBase)
		{
			switch (type)
			{
			case FILETYPE_FILE:
			{
				CFile* pFile = dynamic_cast<CFile*>(pBase);
				Serialize(os, pFile);
			}
				break;
			case FILETYPE_DIR:
			{
				CDir* pDir = dynamic_cast<CDir*>(pBase);
				Serialize(os, pDir);
			}
				break;
			case FILETYPE_LINK:
			{
				CLink* pLink = dynamic_cast<CLink*>(pBase);
				Serialize(os, pLink);
			}
				break;
			default:
				break;
			}
		}
	}
}

template<typename T>
void CBaseSerialize::DeSerialize(std::ifstream& is, T& value)
{
	is.read(reinterpret_cast<char*>(&value), sizeof(T));
#if DEBUGSERIALIZE
	std::cout << "line:" << __LINE__ << "  value:" << value << std::endl;
#endif
}

template<typename VALUE, typename SIZE>
void CBaseSerialize::DeSerialize(std::ifstream& is, VALUE* value, SIZE size)
{
	if (!value || size <= 0)
		return;
	is.read(reinterpret_cast<char*>(value), sizeof(VALUE) * size);
#if DEBUGSERIALIZE
	std::cout << "line:" << __LINE__ << "  value:" << value << "  size:" << size << std::endl;
#endif
}

void CBaseSerialize::DeSerialize(std::ifstream& is, std::string& value)
{
	size_t size = 0;
	DeSerialize(is, size);
	
	char* data = nullptr;
	try
	{
		data = new char[size + 1];
		if (!data)
			throw "";
	}
	catch (...)
	{
		std::cout << "out of memory" << std::endl;
		exit(0);
	}
	memset(data, 0, size + 1);
	DeSerialize(is, data, size);
	
	value = data;
	if (data)
		delete[] data, data = nullptr;

#if DEBUGSERIALIZE
	std::cout << "line:" << __LINE__ << "  value:" << value.data() << "  size:" << value.size() << std::endl;
#endif
}

void CBaseSerialize::DeSerialize(std::ifstream& is, CBase* pBase) 
{
	std::string path;
	DeSerialize(is, path);
	pBase->SetAbsolutePath(path);

	std::string name;
	DeSerialize(is, name);
	pBase->SetName(name);

	int type;
	DeSerialize(is, type);
	pBase->SetFileType(type);

	time_t time;
	DeSerialize(is, time);
	pBase->SetTime(time);
}

void CBaseSerialize::DeSerialize(std::ifstream& is, CFile* pFile) 
{
	DeSerialize(is, (CBase*)pFile);

	unsigned int size = 0;
	DeSerialize(is, size);
	pFile->SetSize(size);

	char* data = nullptr;
	try
	{
		data = new char[size];
		if (!data)
			throw "";
	}
	catch (...)
	{
		std::cout << "out of memory" << std::endl;
		exit(0);
	}
	memset(data, 0, size);
	DeSerialize(is, data, size);
	pFile->SetData(data, size);
	if (data)
		delete[] data, data = nullptr;
}

void CBaseSerialize::DeSerialize(std::ifstream& is, CDir* pDir) 
{
	DeSerialize(is, (CBase*)pDir);

	DeSerialize(is, pDir->m_mapChild, pDir);
}

void CBaseSerialize::DeSerialize(std::ifstream& is, CLink* pLink) 
{
	DeSerialize(is, (CBase*)pLink);

	std::string linkname;
	DeSerialize(is, linkname);
	pLink->SetLinkName(linkname);
}

void CBaseSerialize::DeSerialize(std::ifstream& is, std::map <std::string, CBase*>& child, CBase* pParent)
{
	size_t size = 0;
	DeSerialize(is, size);

	for (size_t i = 0; i < size; i++)
	{
		std::string first;
		DeSerialize(is, first);

		int type = -1;
		DeSerialize(is, type);
		switch (type)
		{
		case FILETYPE_FILE:
		{
			CFile* pFile = new CFile(pParent, first, 0, 0, nullptr);
			DeSerialize(is, pFile);
			child[first] = pFile;
		}
			break;
		case FILETYPE_DIR:
		{
			CDir* pDir = new CDir(pParent, first, 0);
			DeSerialize(is, pDir);
			child[first] = pDir;
		}
			break;
		case FILETYPE_LINK:
		{
			CLink* pLink = new CLink(pParent, first, 0);
			DeSerialize(is, pLink);
			child[first] = pLink;
		}
			break;
		default:
			break;
		}
	}
}

