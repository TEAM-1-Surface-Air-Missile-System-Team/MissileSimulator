#pragma once
#include "ControlManager.h"
#include "target_gen.h"
#include "coordinate_transform.h"
#include <nFramework/util/IniHandler.h>

using namespace nframework;
using namespace nom;
using namespace std::chrono;

/************************************************************************
	Constructor / Destructor
************************************************************************/
ControlManager::ControlManager(void) {
	init();
}

ControlManager::~ControlManager(void) {
	release();
}

/************************************************************************
	initialize / release
************************************************************************/
void
ControlManager::init() {

	ntcout << _T("[") << _T(__FUNCTION__) << _T("] ") << std::endl;


	setUserName(_T("ControlManager"));

	messageArr = {
		{3001, 3002, _T("MSSScenarioAck"), NEED_ACK},//MSSScenarioInfo
		{3004, 0,    _T(""), GIVE_ACK}, // MSSStateAck
		{3005, 3006, _T("StartMSSSimulationAck"), NEED_ACK},//StartMSSSimulation
		{3007, 3008, _T("StopMSSSimulationAck"), NEED_ACK},//StopMSSSimulation
		{6001, 0,    _T(""), GIVE_DATA},//RDSTargetInfoToMss
		{7001, 7004, _T("MissileLoadCommandAckToMSS"), NEED_ACK},//MissileLoadCommandToMSS
		{7002, 7005, _T("MissileControlCommandAckToMSS"), NEED_ACK},//MissileControlCommandToMSS
	};

	// by contract
	mec = new MECComponent;
	mec->setUser(this);
}

void
ControlManager::release() {
	delete mec;
	mec = nullptr;
	meb = nullptr;
}

/************************************************************************
	Inherit Function
************************************************************************/
std::shared_ptr<NOM>
ControlManager::registerMsg(tstring msgName) {
	ntcout << _T("[") << _T(__FUNCTION__) << _T("] ") << msgName << std::endl;

	std::shared_ptr<NOM> nomMsg = mec->registerMsg(msgName);
	registeredMsg.emplace(nomMsg->getInstanceID(), nomMsg);

	return nomMsg;
}

void
ControlManager::discoverMsg(std::shared_ptr<NOM> nomMsg) {
	ntcout << _T("[") << _T(__FUNCTION__) << _T("] ") << nomMsg->getName() << std::endl;


	discoveredMsg.emplace(nomMsg->getInstanceID(), nomMsg);
}

void
ControlManager::updateMsg(std::shared_ptr<NOM> nomMsg) {
	mec->updateMsg(nomMsg);
}

void
ControlManager::reflectMsg(std::shared_ptr<NOM> nomMsg) {
	// if need be, write your code
	auto id = nomMsg->getMessageID();
	Message message = messageArr[getIndex(id)];

	shared_ptr<NOM> sMsg;
	NUShort result = 1;
	NUShort missileID = 1;

	switch (message.kind) {
	case NEED_ACK:
		sMsg = createAck(message.sid, message.sname);

		sMsg->setValue(_T("Result"), &result);
		sMsg->setValue(_T("MissileID"), &missileID);

		this->sendMsg(sMsg);
		break;
	case NEED_DATA:


		break;
	case GIVE_ACK:
		break;
	case GIVE_DATA:
		break;
	default:
		break;
	}
}

void
ControlManager::deleteMsg(std::shared_ptr<NOM> nomMsg) {
	ntcout << _T("[") << _T(__FUNCTION__) << _T("] ") << nomMsg->getName() << std::endl;


	mec->deleteMsg(nomMsg);
	registeredMsg.erase(nomMsg->getInstanceID());
}

void
ControlManager::removeMsg(std::shared_ptr<NOM> nomMsg) {
	ntcout << _T("[") << _T(__FUNCTION__) << _T("] ") << nomMsg->getName() << std::endl;


	std::map<unsigned int, std::shared_ptr<NOM>>::iterator itr;
	itr = discoveredMsg.find(nomMsg->getInstanceID());

	if (itr != discoveredMsg.end()) {
		discoveredMsg.erase(nomMsg->getInstanceID());
	}
	else {
		tcerr << _T("[ControlManager] ") << _T("message was not removed.") << std::endl;
	}
}

void
ControlManager::sendMsg(std::shared_ptr<NOM> nomMsg) {
	tcout << "ControlManager sendMsg" << std::endl;
	mec->sendMsg(nomMsg);
}

shared_ptr<NOM>
ControlManager::createAck(NUShort id, tstring name) {
	auto ack = meb->getNOMInstance(this->getUserName(), name);

	NUShort msgID = id;
	ack->setValue(_T("messageHeader.MessageID"), &msgID);
	NUShort msgLen = ack->getLength();
	ack->setValue(_T("messageHeader.MessageLength"), &msgLen);

	return ack;
}

int
ControlManager::getIndex(NUShort id) {
	Message message;
	for (int i = 0; i < messageArr.size(); i++) {
		if (messageArr[i].rid == id.getValue()) {
			return i;
		}
	}
	return -1;
}

void
ControlManager::FlySimulate() {
	double dist_to_ATS;
	while (1) {
		if (missile.Tgt_Alt < 15000.0) {
			missile = CV_manuever(missile, 0.1);
		}
		else {
			missile = guidance_ECEF_only(missile, ATS, 0.1, 20.0, 10.0);
		}
		dist_to_ATS = calc_dist(missile.ECEF_x, missile.ECEF_y, missile.ECEF_z, ATS.ECEF_x, ATS.ECEF_y, ATS.ECEF_z);

		if (dist_to_ATS < 100.0 || isSimulation == false) {
			break;
		}
	}
	isLaunch = false;
}


void
ControlManager::recvMsg(std::shared_ptr<NOM> nomMsg) {
	ntcout << _T("[") << _T(__FUNCTION__) << _T("] ") << nomMsg->getName() << std::endl;

	auto id = nomMsg->getMessageID();
	Message message = messageArr[getIndex(id)];



	if (id == 3001) { // MSSScenarioInfo
		ntcout << _T("Success! MSSScenarioInfo received! ") << std::endl;

		missile.Tgt_Lat = nomMsg->getValue(_T("MissileFlyState.Tgt_Lat"))->toDouble();
		missile.Tgt_Lon = nomMsg->getValue(_T("MissileFlyState.Tgt_Lon"))->toDouble();
		missile.Tgt_Alt = nomMsg->getValue(_T("MissileFlyState.Tgt_Alt"))->toDouble();

		missile.v0 = nomMsg->getValue(_T("MissileFlyState.v0"))->toDouble();

		missile.ECEF_x = nomMsg->getValue(_T("MissileFlyState.ECEF_x"))->toDouble();
		missile.ECEF_y = nomMsg->getValue(_T("MissileFlyState.ECEF_y"))->toDouble();
		missile.ECEF_z = nomMsg->getValue(_T("MissileFlyState.ECEF_z"))->toDouble();

		missile.ECEF_velo_x = nomMsg->getValue(_T("MissileFlyState.ECEF_velo_x"))->toDouble();
		missile.ECEF_velo_y = nomMsg->getValue(_T("MissileFlyState.ECEF_velo_y"))->toDouble();
		missile.ECEF_velo_z = nomMsg->getValue(_T("MissileFlyState.ECEF_velo_z"))->toDouble();

		missile.velo_x = nomMsg->getValue(_T("MissileFlyState.velo_x"))->toDouble();
		missile.velo_y = nomMsg->getValue(_T("MissileFlyState.velo_y"))->toDouble();
		missile.velo_z = nomMsg->getValue(_T("MissileFlyState.velo_z"))->toDouble();

		missile.roll_angle = nomMsg->getValue(_T("MissileFlyState.roll_angle"))->toDouble();
		missile.yaw_angle = nomMsg->getValue(_T("MissileFlyState.yaw_angle"))->toDouble();
		missile.pitch_angle = nomMsg->getValue(_T("MissileFlyState.pitch_angle"))->toDouble();

		missile.max_time = nomMsg->getValue(_T("MissileFlyState.max_time"))->toDouble();


	}
	else if (id == 3004) { // MSSStateInfoAck
		ntcout << _T("Success! MSSStateInfoAck received! ") << std::endl;
	}
	else if (id == 3005) { // StartMSSSimulation
		isSimulation = true;
		ntcout << _T("Success! StartMSSSimulation received! ") << std::endl;
		auto MSSFlyStateNOM = this->registerMsg(_T("MSSStateInfo"));
		// 미사일 초기정보(시나리오로 들어온 값 대입)
	}
	else if (id == 3007) { // StopMSSSimulation
		ntcout << _T("Success! StopMSSSimulation received! ") << std::endl;
		isSimulation = false;
	}
	else if (id == 6001) { //RDSTargetInfoToMss
		ntcout << _T("Success! MSSScenarioInfo received! ") << std::endl;

		ATS.Tgt_Lat = nomMsg->getValue(_T("TargetFlyState.Tgt_Lat"))->toDouble();
		ATS.Tgt_Lon = nomMsg->getValue(_T("TargetFlyState.Tgt_Lon"))->toDouble();
		ATS.Tgt_Alt = nomMsg->getValue(_T("TargetFlyState.Tgt_Alt"))->toDouble();

		ATS.v0 = nomMsg->getValue(_T("TargetFlyState.v0"))->toDouble();

		ATS.ECEF_x = nomMsg->getValue(_T("TargetFlyState.ECEF_x"))->toDouble();
		ATS.ECEF_y = nomMsg->getValue(_T("TargetFlyState.ECEF_y"))->toDouble();
		ATS.ECEF_z = nomMsg->getValue(_T("TargetFlyState.ECEF_z"))->toDouble();

		ATS.ECEF_velo_x = nomMsg->getValue(_T("TargetFlyState.ECEF_velo_x"))->toDouble();
		ATS.ECEF_velo_y = nomMsg->getValue(_T("TargetFlyState.ECEF_velo_y"))->toDouble();
		ATS.ECEF_velo_z = nomMsg->getValue(_T("TargetFlyState.ECEF_velo_z"))->toDouble();

		ATS.velo_x = nomMsg->getValue(_T("TargetFlyState.velo_x"))->toDouble();
		ATS.velo_y = nomMsg->getValue(_T("TargetFlyState.velo_y"))->toDouble();
		ATS.velo_z = nomMsg->getValue(_T("TargetFlyState.velo_z"))->toDouble();

		ATS.roll_angle = nomMsg->getValue(_T("TargetFlyState.roll_angle"))->toDouble();
		ATS.yaw_angle = nomMsg->getValue(_T("TargetFlyState.yaw_angle"))->toDouble();
		ATS.pitch_angle = nomMsg->getValue(_T("TargetFlyState.pitch_angle"))->toDouble();

		ATS.max_time = nomMsg->getValue(_T("TargetFlyState.max_time"))->toDouble();

	}
	else if (id == 7001) { // MissileLoadCommandToMSS
		ntcout << _T("Success! MissileLoadCommand received! ") << std::endl;
	}
	else if (id == 7002) { // MissileControlCommandToMSS
		ntcout << _T("Success! MissileLaunch received! ") << std::endl;

		if (isLaunch == false) {
			isLaunch = true;
			thread t(&ControlManager::FlySimulate, this);
		}
	}

	shared_ptr<NOM> sMsg;
	NUShort result = 1;
	NUShort missileID = 1;

	switch (message.kind) {
	case NEED_ACK:
		sMsg = createAck(message.sid, message.sname);

		sMsg->setValue(_T("Result"), &result);
		sMsg->setValue(_T("MissileID"), &missileID);

		this->sendMsg(sMsg);
		break;
	case NEED_DATA:

		break;
	case GIVE_ACK:
		break;
	case GIVE_DATA:
		break;
	default:
		break;
	}

}

void
ControlManager::setUserName(tstring userName) {
	name = userName;
}

tstring
ControlManager::getUserName() {
	return name;
}

void
ControlManager::setData(void* data) {
	// if need be, write your code
}

bool
ControlManager::start() {
	tstring cmdStr;

	nTimer = &(NTimer::getInstance());
	auto timerHandle = 0;

	STDFUNCTION testPeriodic = std::bind(&ControlManager::testSend, this);
	timerHandle = nTimer->addPeriodicTask(2000, testPeriodic);

	while (tcin >> cmdStr) {}

	return true;
}

bool
ControlManager::stop() {
	return true;
}

void
ControlManager::setMEBComponent(IMEBComponent* realMEB) {
	meb = realMEB;
	mec->setMEB(meb);
}

void
ControlManager::testSend() {
	std::shared_ptr<NOM> testIntrNOM = meb->getNOMInstance(getUserName(), _T("MSSStateInfo"));
	NUShort id = 6002;
	NUShort len = testIntrNOM->getLength();
	testIntrNOM->setValue(_T("MessageHeader.MessageID"), &id);
	testIntrNOM->setValue(_T("MessageHeader.MessageLength"), &id);

	this->sendMsg(testIntrNOM);
}

/************************************************************************
	Export Function
************************************************************************/
extern "C" BASEMGRDLL_API
BaseManager* createObject() {
	return new ControlManager;
}

extern "C" BASEMGRDLL_API
void deleteObject(BaseManager* userManager) {
	delete userManager;
}

