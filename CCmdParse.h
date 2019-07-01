#ifndef __CCMDPARSE_H__
#define __CCMDPARSE_H__

#include <string>
#include <vector>

class CCmdParse 
{
public:
	CCmdParse();
	~CCmdParse();

private:
	void RightTrim(std::string& strBuf);
	void LeftTrim(std::string& strBuf);
	void TrimSpace(std::string& strBuf);
	int FindSeparator(std::string& strCmd);
	int FindCmdSeparator(std::string& strCmd);
	int FindCmdType(std::string& strCmd, std::string& strCmdType);
	int FindOptionsAndParams(std::string& strCmd, std::vector<std::string>& vOption, std::vector<std::string>& vParam);

public:
	int Parse(std::string strCmd
		, std::string& strCmdType
		, std::vector<std::string>& vOption
		, std::vector<std::string>& vParam
	);
};

#endif
