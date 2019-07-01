#ifndef __SERIALIZE_H__
#define __SERIALIZE_H__
#include "CFileStat.h"
#include <fstream>

class CBaseSerialize 
{
public:
	CBaseSerialize();
	~CBaseSerialize();

private:
	template<typename T>
	void Serialize(std::ofstream& os, T& value);

	template<typename VALUE, typename SIZE>
	void Serialize(std::ofstream& os, VALUE* value, SIZE size);

	void Serialize(std::ofstream& os, std::string value);

	void Serialize(std::ofstream& os, CBase* pBase);

	void Serialize(std::ofstream& os, CFile* pFile);

	void Serialize(std::ofstream& os, CLink* pLink);

	void Serialize(std::ofstream& os, std::map <std::string, CBase*>& child);

	template<typename T>
	void DeSerialize(std::ifstream& is, T& value);

	template<typename VALUE, typename SIZE>
	void DeSerialize(std::ifstream& is, VALUE* value, SIZE size);

	void DeSerialize(std::ifstream& is, std::string& value);

	void DeSerialize(std::ifstream& is, CBase* pBase);

	void DeSerialize(std::ifstream& is, CFile* pFile);
	
	void DeSerialize(std::ifstream& is, CLink* pLink);
	
	void DeSerialize(std::ifstream& is, std::map <std::string, CBase*>& child, CBase* parent);

public:
	void Serialize(std::ofstream& os, CDir* pDir);

	void DeSerialize(std::ifstream& is, CDir* pDir);
};

#endif
