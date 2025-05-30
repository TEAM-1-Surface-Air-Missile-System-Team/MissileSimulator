#include "CommMessageHandler.h"

/************************************************************************
	initialize / release
************************************************************************/
CommMessageHandler::CommMessageHandler() {
	initialize();
}

CommMessageHandler::~CommMessageHandler() {
	release();
}

void
CommMessageHandler::initialize() {
	// if need be, write your code
	setIDNameTable(3001, _T("MSSScenarioInfo"));
	setIDNameTable(3002, _T("MSSScenarioAck"));
	setIDNameTable(3003, _T("MSSStateInfo"));
	setIDNameTable(3004, _T("MSSStateAck"));
	setIDNameTable(3005, _T("StartMSSSimulation"));
	setIDNameTable(3006, _T("StartMSSSimulationAck"));
	setIDNameTable(3007, _T("StopMSSSimulation"));
	setIDNameTable(3008, _T("StopMSSSimulationAck"));

	setIDNameTable(6001, _T("RDSTargetInfoToMss"));
	setIDNameTable(6002, _T("MSSStateInfoToRDS"));

	setIDNameTable(7001, _T("MissileLoadCommand"));
	setIDNameTable(7002, _T("MissileControlCommandToMSS"));
	setIDNameTable(7003, _T("MissileStatus"));
	setIDNameTable(7004, _T("MissileLoadCommandAck"));
	setIDNameTable(7005, _T("MissileControlCommandAck"));
	setIDNameTable(7006, _T("MissileStatusACK"));

}

void
CommMessageHandler::release() {
	idNameTable.clear();
}

/************************************************************************
	ID_Name table management
************************************************************************/
void
CommMessageHandler::setIDNameTable(unsigned short msgID, tstring msgName) {
	idNameTable.insert(pair<unsigned short, tstring>(msgID, msgName));
}

tstring
CommMessageHandler::getMsgName(unsigned short msgID) {
	tstring msgName;
	if (auto itr = idNameTable.find(msgID); itr != idNameTable.end()) {
		msgName = itr->second;
	}
	else {
		msgName = _T("");
	}

	return msgName;
}