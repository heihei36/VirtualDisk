// VirtaulDisk.cpp : �������̨Ӧ�ó������ڵ㡣
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

