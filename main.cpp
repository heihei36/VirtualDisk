// VirtaulDisk.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "CDiskManager.h"
#include <vector>
#include <iostream>
using namespace std;

int main()
{
	CDiskManager* pManager = CDiskManager::GetInstance();
	const char* pDrive = DEFAULT_DRIVE;
	pManager->Init(pDrive);

	pManager->DoWork();
    return 0;
}

