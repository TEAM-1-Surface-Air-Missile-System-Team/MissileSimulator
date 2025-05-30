#pragma once
#include <nFramework/BaseManager.h>
#include <nFramework/mec/MECComponent.h>
#include <nFramework/nom/NOMMain.h>
#include <nFramework/nTimer/NTimer.h>
#include <nFramework/nLineStream/NLineStreamMain.h>
#include "coordinate_transform.h"
#include "target_gen.h"
#include <unordered_map>
#include <sstream>
#include <thread>
#include <chrono>
//$(SolutionDir)lib;$(NFW_DIR)\lib;

//$(NFW_DIR)\lib;$(SolutionDir)lib;%(AdditionalLibraryDirectories)

using namespace nframework;
using namespace std;
using namespace nom;
using namespace nlinestream;


class BASEMGRDLL_API ControlManager : public BaseManager {
public:
	ControlManager(void);
	~ControlManager(void);

public:
	// inherited from the BaseManager class
	virtual std::shared_ptr<NOM> registerMsg(tstring) override;
	virtual void discoverMsg(std::shared_ptr<NOM>) override;
	virtual void updateMsg(std::shared_ptr<NOM>) override;
	virtual void reflectMsg(std::shared_ptr<NOM>) override;
	virtual void deleteMsg(std::shared_ptr<NOM>) override;
	virtual void removeMsg(std::shared_ptr<NOM>) override;
	virtual void sendMsg(std::shared_ptr<NOM>) override;
	virtual void recvMsg(std::shared_ptr<NOM>) override;
	virtual void setUserName(tstring) override;
	virtual tstring getUserName() override;
	virtual void setData(void*) override;
	virtual bool start() override;
	virtual bool stop() override;
	virtual void setMEBComponent(IMEBComponent*) override;

private:
	void init();
	void release();

	void testSend();
	void FlySimulate();
	void countDown();

	int getIndex(NUShort id);
	shared_ptr<NOM> createAck(NUShort id, tstring name);
	enum MessageKind {
		NEED_ACK, NEED_DATA, GIVE_ACK, GIVE_DATA
	};
	struct Message {
		int rid;
		int sid;
		tstring sname;
		MessageKind kind;
	};
	struct FlyStateStruct {
		double Tgt_Lat;
		double Tgt_Lon;
		double Tgt_Alt;
		double v0;
		double ECEF_x;
		double ECEF_y;
		double ECEF_z;
		double velo_x;
		double velo_y;
		double velo_z;
		double roll_angle;
		double yaw_angle;
		double pitch_angle;
		double max_time;
	};

	bool isSimulation = false;
	bool isLaunch = false;
	Tgt missile, ATS;

	vector<Message> messageArr;
	FlyStateStruct missileFlyState;
private:
	IMEBComponent* meb;
	MECComponent* mec;
	tstring name;
	std::map<unsigned int, std::shared_ptr<NOM>> registeredMsg;
	std::map<unsigned int, std::shared_ptr<NOM>> discoveredMsg;

	std::shared_ptr<NOM> testObjNOM;
	NTimer* nTimer;

	std::vector<tstring> weapons;


	NLineTstream ntcout{ Level::COUT };

};

