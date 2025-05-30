#pragma once
#include <map>
#include <nFramework/util/util.h>
using namespace std;
class CommMessageHandler
{
public:
	CommMessageHandler();
	~CommMessageHandler();

	tstring getMsgName(unsigned short msgID);
	void setIDNameTable(unsigned short msgID, tstring msgName);

private:
	void initialize();
	void release();

private:
	map<unsigned short, tstring> idNameTable;
};

