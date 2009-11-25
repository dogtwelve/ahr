#ifndef _TOUCHZONES_H_
#define _TOUCHZONES_H_

#include "Config.h"
#include "Singleton.h"

#ifdef DRAW_TOUCH_ZONES
	#include "Lib2D.h"
#endif // DRAW_TOUCH_ZONES

#ifdef USE_SYNCHRONIZE_TOUCH

#include <pthread.h>

extern "C" void lockTouchSyncObj();
extern "C" void unlockTouchSyncObj();
extern "C" void destroyTouchSyncObj();

#endif //USE_SYNCHRONIZE_TOUCH



class ITouchZonesListener
{
public:
	virtual void ZonePressed(short zoneId) = 0;
	virtual void ZoneReleased(short zoneId) = 0;
	virtual void ZoneActivated(short zoneId) = 0;
	virtual void ZoneMove(short zoneId, int x, int y) = 0;
};

class CTouchZones : public CSingleton<k_nSingletonTouchZones, CTouchZones>
{
public:
	unsigned char crtTouchId;
	short crtTouchX, crtTouchY;
	
	CTouchZones();
	~CTouchZones();

	void Update();

	short AddZone(unsigned char id, short x1, short x2, short y1, short y2);
	short AddZoneWithMoveDetect(unsigned char id, short x1, short x2, short y1, short y2);
	void ClearZones();
	void ClearZone(int zoneId);
	
	bool WasZoneActivated(unsigned char zoneId);	
	void ClearLastZoneActivated();

	bool IsZonePressed(unsigned char zoneId);
	bool IsZoneReleased(unsigned char zoneId);

	void SetListener(ITouchZonesListener* lst);
	ITouchZonesListener* GetListener() { return listener; }

	void TouchPressed(unsigned char touchId, short x, short y);
	void TouchReleased(unsigned char touchId, short x, short y);
	void TouchMoved(unsigned char touchId, short x, short y);

	void ForceReleaseZones(bool bIgnoreNextRelease = false);

	void WillIgnoreNextRelease();

#ifdef DRAW_TOUCH_ZONES
	void DrawZones(CLib2D& lib2D);
#endif // DRAW_TOUCH_ZONES

private:
	const static int MAX_ZONES = 64;
	const static int MAX_TOUCHES = 10;
	const static int MAX_EVENTS = 50;

	ITouchZonesListener* listener;

	short* ZonesPos;
	short ZonesNo;

	short* TouchLastPos;

	short* TouchEvents;
	short TouchEventsIdx1, TouchEventsIdx2; // queue style indexes, first is for push and second is for pop
	
	short LastActivatedZoneId;

	//bool bIgnoreNextRelease; // fix for #1966566
	
	unsigned char* pressedZones;

	bool IsZoneIntersected(unsigned char zoneIdx, short x, short y);
	short GetNextIntersectedZone(short x, short y, short crtZoneIdx);


#ifdef USE_CHEAT_ZONES
public:
	static const int k_CHEAT_NONE = 0;

	//menu cheats
	static const int k_CHEAT_UNLOCK_ALL = 1;
	static const int k_CHEAT_MONEY	= 2;

	//ingame cheats
	static const int k_CHEAT_WIN_RACE = 2;
	static const int k_CHEAT_LOOSE_RACE = 3;
	static const int k_CHEAT_GAME_COMPLETE = 4;
	
	static const int k_CHEAT_VIEW_FRAMES = 100;

	static const int k_CHEAT_TIMEMS = 2000;
	
	static unsigned short s_cheatName[];
	static int s_nCheatID;
	static int s_showCheatCounter;

	static short s_nCheatZoneID; // 0, 1 only 2 zones
	static unsigned int s_nCheatLastTimeInZone;
	
	
	void updateCheatZones();
	void checkCheatZones(bool ingame);
	void drawActivatedCheat();

	static void setCheatName();
	

#endif //USE_CHEAT_ZONES
};

#endif // _TOUCHZONES_H_
