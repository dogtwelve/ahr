
#include <string.h>
#include "Config.h"

#ifdef USE_TOUCH_SCREEN

#include "TouchZones.h"

#include "memoryallocation.h"

#include "HG/HighGear.h"
#include "str_utils.h"

#define ZONE_PARAMS 5
#define ZONE_ID(z) ZonesPos[(z * ZONE_PARAMS)    ]
#define ZONE_X1(z) ZonesPos[(z * ZONE_PARAMS) + 1]
#define ZONE_X2(z) ZonesPos[(z * ZONE_PARAMS) + 2]
#define ZONE_Y1(z) ZonesPos[(z * ZONE_PARAMS) + 3]
#define ZONE_Y2(z) ZonesPos[(z * ZONE_PARAMS) + 4]

#define ZONE_ID_MASK		0xFF

#define FLAG_INZONE_MOVE	(1 << 8)

#define EVENT_PRESS			0x0000
#define EVENT_MOVE			0x0100
#define EVENT_RELEASE		0x0200

#define EVENT_TYPE_MASK 	0x0F00

enum {
	ZONE_NOT_PRESSED = 0,
	ZONE_PRESSED,
	ZONE_IGNORE_NEXT_RELESE,
};

void debug_out(const char* x, ...);

#ifdef USE_SYNCHRONIZE_TOUCH

pthread_mutex_t g_touchMutex;

extern "C" void lockTouchSyncObj() 
{ 
	static bool bInitiliazedSyncObj = false;
	
	if(!bInitiliazedSyncObj)
	{
		pthread_mutex_init(&g_touchMutex, NULL);
		bInitiliazedSyncObj = true;
	}
	
	pthread_mutex_lock(&g_touchMutex);
	//printf("lock touch \n");		
}

extern "C" void unlockTouchSyncObj() 
{ 
	//printf("unlock touch\n");
	pthread_mutex_unlock(&g_touchMutex);
}

extern "C" void destroyTouchSyncObj()
{
	pthread_mutex_destroy(&g_touchMutex);
}

#endif //USE_SYNCHRONIZE_TOUCH


// constructor
CTouchZones::CTouchZones()
{
	ZonesPos = NEW short[MAX_ZONES * ZONE_PARAMS];
	ZonesNo = 0;

	TouchEvents = NEW short[MAX_EVENTS * 3];
	TouchEventsIdx1 = 0;
	TouchEventsIdx2 = 0;

	TouchLastPos = NEW short[2 * MAX_TOUCHES];

	listener = 0;
	
	LastActivatedZoneId = -1;

//#ifdef DRAW_TOUCH_ZONES
	pressedZones = NEW unsigned char[MAX_ZONES];
	memset(pressedZones, 0, sizeof(unsigned char) * MAX_ZONES);
//#endif // DRAW_TOUCH_ZONES

//	bIgnoreNextRelease = false;
}

// adds a rectangulat touch zone
short CTouchZones::AddZone(unsigned char id, short x1, short y1, short x2, short y2)
{
	A_ASSERT(ZonesNo < MAX_ZONES);

	if (ZonesNo < MAX_ZONES)
	{
		ZONE_ID(ZonesNo) = (unsigned short)id;
		ZONE_X1(ZonesNo) = x1;
		ZONE_X2(ZonesNo) = x2;
		ZONE_Y1(ZonesNo) = y1;
		ZONE_Y2(ZonesNo) = y2;

		return ZonesNo++;
	}

	return -1;
}

short CTouchZones::AddZoneWithMoveDetect(unsigned char id, short x1, short y1, short x2, short y2)
{
	short zone = AddZone(id, x1, y1, x2, y2);
	ZONE_ID(zone) |= FLAG_INZONE_MOVE;
	return zone;
}

// deletes all zones
void CTouchZones::ClearZones()
{
	ZonesNo = 0;
}

void CTouchZones::ClearZone(int zoneId)
{
	for (int z = ZonesNo - 1; z >= 0; z--)
	{
		if ((ZONE_ID(z) & ZONE_ID_MASK) == zoneId)
		{
			ZONE_X1(z) = -1;
			ZONE_X2(z) = -1;
			ZONE_Y1(z) = -1;
			ZONE_Y2(z) = -1;

			break;
		}
	}
}

void CTouchZones::SetListener(ITouchZonesListener* lst)
{
	this->listener = lst;
}

// check if the zone was just activated
bool CTouchZones::WasZoneActivated(unsigned char zoneId)
{
//#ifdef IPHONE
//	LOGDEBUG("CTouchZones::WasZoneActivated - LastActivatedZoneId[%d], zoneId[%d]\n", 
//			 LastActivatedZoneId, zoneId);
//#endif
	
	return LastActivatedZoneId == zoneId;
}

bool CTouchZones::IsZonePressed(unsigned char zoneId)
{
	for (int i=0; i<ZonesNo; i++)
	{
		int z = ZONE_ID(i) & ZONE_ID_MASK;

		if (z == zoneId && pressedZones[i] == ZONE_PRESSED)
			return true;
	}

	return false;
}

void CTouchZones::ClearLastZoneActivated()
{
	LastActivatedZoneId = -1;
}

// zones intersections tests
bool CTouchZones::IsZoneIntersected(unsigned char zoneIdx, short x, short y)
{
	return ZONE_X1(zoneIdx) <= x && x <= ZONE_X2(zoneIdx) && ZONE_Y1(zoneIdx) <= y && y <= ZONE_Y2(zoneIdx);
}

short CTouchZones::GetNextIntersectedZone(short x, short y, short crtZoneIdx)
{
	if (crtZoneIdx < 0)
		crtZoneIdx = ZonesNo - 1;
	else
	{
		if (crtZoneIdx >= ZonesNo)
			crtZoneIdx = ZonesNo;

		crtZoneIdx--;
	}

	for (int z = crtZoneIdx; z >= 0; z--)
		if (ZONE_X1(z) <= x && x <= ZONE_X2(z) && ZONE_Y1(z) <= y && y <= ZONE_Y2(z))
			return z;
	
	return -1;
}

// receiving events
void CTouchZones::TouchPressed(unsigned char touchId, short x, short y)
{
	if (TouchEventsIdx1 < MAX_EVENTS)
	{
		TouchEvents[3 * TouchEventsIdx1    ] = touchId | EVENT_PRESS;
		TouchEvents[3 * TouchEventsIdx1 + 1] = x;
		TouchEvents[3 * TouchEventsIdx1 + 2] = y;
		TouchEventsIdx1++;
	}
}

void CTouchZones::TouchMoved(unsigned char touchId, short x, short y)
{
	if (TouchEventsIdx1 < MAX_EVENTS)
	{
		TouchEvents[3 * TouchEventsIdx1    ] = touchId | EVENT_MOVE;
		TouchEvents[3 * TouchEventsIdx1 + 1] = x;
		TouchEvents[3 * TouchEventsIdx1 + 2] = y;
		TouchEventsIdx1++;
	}
}

void CTouchZones::TouchReleased(unsigned char touchId, short x, short y)
{
	if (TouchEventsIdx1 < MAX_EVENTS)
	{
		TouchEvents[3 * TouchEventsIdx1    ] = touchId | EVENT_RELEASE;
		TouchEvents[3 * TouchEventsIdx1 + 1] = x;
		TouchEvents[3 * TouchEventsIdx1 + 2] = y;
		TouchEventsIdx1++;
	}
}

// to be called once in each frame
// reads the received events and delivering events
void CTouchZones::Update()
{
#ifdef USE_SYNCHRONIZE_TOUCH
	lockTouchSyncObj();
#endif

	short zone, lastX, lastY;
	
	LastActivatedZoneId = -1;
	
	TouchEventsIdx2 = TouchEventsIdx1;
	TouchEventsIdx1 = 0;

#ifdef USE_CHEAT_ZONES
	updateCheatZones();
#endif
	
	// checking events
	for (int e = 0; e < TouchEventsIdx2; e++)
	{
		crtTouchId = TouchEvents[3 * e] & ZONE_ID_MASK;
		crtTouchX = TouchEvents[3 * e + 1];
		crtTouchY = TouchEvents[3 * e + 2];
		lastX = TouchLastPos[2 * crtTouchId];
		lastY = TouchLastPos[2 * crtTouchId + 1];

		switch (TouchEvents[3 * e] & EVENT_TYPE_MASK)
		{
			case EVENT_PRESS:
			{
				//bIgnoreNextRelease = false;

				zone = -1;
				while ((zone = GetNextIntersectedZone(crtTouchX, crtTouchY, zone)) >= 0)
				{
					if (listener)
					{
						listener->ZonePressed(ZONE_ID(zone) & ZONE_ID_MASK);
						
						if (ZONE_ID(zone) & FLAG_INZONE_MOVE)
							listener->ZoneMove(ZONE_ID(zone) & ZONE_ID_MASK, crtTouchX - ZONE_X1(zone), crtTouchY - ZONE_Y1(zone));
					}
					
					pressedZones[zone] = ZONE_PRESSED;
				}
				
				TouchLastPos[2 * crtTouchId] = crtTouchX;
				TouchLastPos[2 * crtTouchId + 1] = crtTouchY;
				break;
			}

			case EVENT_MOVE:
			{
				zone = -1;
				while ((zone = GetNextIntersectedZone(lastX, lastY, zone)) >= 0)
				{
					// we were inside a zone, checking if we just got out of it
					if (!IsZoneIntersected(zone, crtTouchX, crtTouchY))
					{
						if (listener)
							listener->ZoneReleased(ZONE_ID(zone) & ZONE_ID_MASK);
						
						pressedZones[zone] = ZONE_NOT_PRESSED;
					}
					else if (ZONE_ID(zone) & FLAG_INZONE_MOVE)
					{
						// we are inside a zone with move detection
						if (listener)
							listener->ZoneMove(ZONE_ID(zone) & ZONE_ID_MASK, crtTouchX - ZONE_X1(zone), crtTouchY - ZONE_Y1(zone));
					}
				}
				
				// checking if we just got in another zone
				zone = -1;
				while ((zone = GetNextIntersectedZone(crtTouchX, crtTouchY, zone)) >= 0)
				{
					if (!IsZoneIntersected(zone, lastX, lastY))
					{
						if (listener)
							listener->ZonePressed(ZONE_ID(zone) & ZONE_ID_MASK);

						pressedZones[zone] = ZONE_PRESSED;
					}
				}

				TouchLastPos[2 * crtTouchId] = crtTouchX;
				TouchLastPos[2 * crtTouchId + 1] = crtTouchY;
				break;
			}

			case EVENT_RELEASE:
			{
				//if (bIgnoreNextRelease)
				//{
				//	bIgnoreNextRelease = false;
				//	break;
				//}

				zone = -1;
				while ((zone = GetNextIntersectedZone(crtTouchX, crtTouchY, zone)) >= 0)
				{
					if (listener)
					{
						if (pressedZones[zone] != ZONE_IGNORE_NEXT_RELESE)
							listener->ZoneActivated(ZONE_ID(zone) & ZONE_ID_MASK);
					}
						
					pressedZones[zone] = ZONE_NOT_PRESSED;
					
					LastActivatedZoneId = ZONE_ID(zone) & ZONE_ID_MASK;	
				}
				break;
			}
		}
	}

#ifdef USE_SYNCHRONIZE_TOUCH	
	unlockTouchSyncObj();
#endif
}

void CTouchZones::ForceReleaseZones(bool bIgnoreNextRelease)
{
	// checking zones
	for (int zone = 0; zone < ZonesNo; zone ++)
	{
		if (pressedZones[zone])
		{
			if (listener)
				listener->ZoneReleased(ZONE_ID(zone) & ZONE_ID_MASK);

			if (bIgnoreNextRelease)
				pressedZones[zone] = ZONE_IGNORE_NEXT_RELESE;
			else
				pressedZones[zone] = ZONE_NOT_PRESSED;
		}
	}

	TouchEventsIdx2 = 0;
	TouchEventsIdx1 = 0;

	ClearZones();
}

//void CTouchZones::WillIgnoreNextRelease()
//{
//	bIgnoreNextRelease = true;
//}

#ifdef DRAW_TOUCH_ZONES
void CTouchZones::DrawZones(CLib2D& lib2D)
{
	int i;
	for (i = 0; i < ZonesNo; i++)
	{
		short color = (pressedZones[i] > 0) ? COLOR_GREEN : COLOR_RED;
		
		lib2D.DrawAlphaColorRect(ZONE_X1(i), ZONE_Y1(i), ZONE_X2(i) - ZONE_X1(i), 7, 0x3000 | color, color);
		lib2D.DrawAlphaColorRect(ZONE_X1(i), ZONE_Y1(i), 7, ZONE_Y2(i) - ZONE_Y1(i), 0x3000 | color, color);
		lib2D.DrawAlphaColorRect(ZONE_X1(i), ZONE_Y2(i) - 7, ZONE_X2(i) - ZONE_X1(i), 7, 0x3000 | color, color);
		lib2D.DrawAlphaColorRect(ZONE_X2(i) - 7, ZONE_Y1(i), 7, ZONE_Y2(i) - ZONE_Y1(i), 0x3000 | color, color);
	}
}
#endif // DRAW_TOUCH_ZONES

//destructor
CTouchZones::~CTouchZones()
{
	SAFE_DELETE(ZonesPos);
	SAFE_DELETE(TouchLastPos);
	SAFE_DELETE(TouchEvents);
//#ifdef DRAW_TOUCH_ZONES
	SAFE_DELETE(pressedZones);
//#endif // DRAW_TOUCH_ZONES
}


#ifdef USE_CHEAT_ZONES
unsigned short CTouchZones::s_cheatName[512];
int CTouchZones::s_nCheatID = k_CHEAT_NONE;
int CTouchZones::s_showCheatCounter = 0;
short CTouchZones::s_nCheatZoneID = -1;
unsigned int CTouchZones::s_nCheatLastTimeInZone = 0;

void CTouchZones::setCheatName()
{
//	if( CHighGear::GetInstance()->m_options.cheat_unlock_all ) 
//		strcpy(s_cheatName, "!!!ENABLE ");
//	else
//		strcpy(s_cheatName, "!!!DISABLE ");

	s_showCheatCounter = k_CHEAT_VIEW_FRAMES;	
	strcat(s_cheatName, "CHEAT UNLOCK ALL!!!");
}

void CTouchZones::updateCheatZones()
{
	CHighGear *hg = CHighGear::GetInstance();
	short cheatZoneID, x, y, lastX, lastY;

	if (s_nCheatZoneID >= 0 && GETTIMEMS() - s_nCheatLastTimeInZone > k_CHEAT_TIMEMS)
	{	
		//mark the cheat activated
//		if(hg->m_state == CHighGear::gs_menu && MenuContainer::GetInstance()->getMenu() == MENU_MAIN_INDEX)
//		{
//			s_nCheatID = k_CHEAT_UNLOCK_ALL + s_nCheatZoneID;
//		}
//		else if(hg->m_state == CHighGear::gs_play)
//		{
//			s_nCheatID = k_CHEAT_WIN_RACE + s_nCheatZoneID;
//		}
//		else
		{
			s_nCheatID = k_CHEAT_NONE;
		}

		s_nCheatZoneID = -1;
		return; // avoid parsing all events							
	}
	
	// checking events
	for (int e = 0; e < TouchEventsIdx2; e++)
	{
		x = TouchEvents[3 * e + 1];
		y = TouchEvents[3 * e + 2];		

		//get cheatZoneID
		int cheatZoneID = -1;	
		if( x > hg->m_dispX - 50 )
		{
			if(y < 50)
				cheatZoneID = 0;
			else if( y > 50  && y < 100)
				cheatZoneID = 1;

			if (y > hg->m_dispY - 50)
				cheatZoneID = 2;
		}

		if(cheatZoneID >= 0)
		{
			switch (TouchEvents[3 * e] & EVENT_TYPE_MASK)
			{
				case EVENT_PRESS:
				{
					if(cheatZoneID != s_nCheatZoneID)
					{
						//reset to new cheat zone
						s_nCheatLastTimeInZone = GETTIMEMS();
						s_nCheatZoneID = cheatZoneID;
						s_nCheatID = k_CHEAT_NONE;
					}
					break;
				}
				case EVENT_RELEASE:
				{
					if(cheatZoneID == s_nCheatZoneID)
					{
						//if( GETTIMEMS()- s_nCheatLastTimeInZone > k_CHEAT_TIMEMS)
						//{	
						//	////mark the cheat activated
						//	//if(hg->m_state == CHighGear::gs_menu && MenuContainer::GetInstance()->getMenu() == MENU_MAIN_INDEX)
						//	//{
						//	//	s_nCheatID = k_CHEAT_UNLOCK_ALL + s_nCheatZoneID;
						//	//}
						//	//else if(hg->m_state == CHighGear::gs_play)
						//	//{
						//	//	s_nCheatID = k_CHEAT_WIN_RACE + s_nCheatZoneID;
						//	//}
						//	//else
						//	//{
						//	//	s_nCheatID = k_CHEAT_NONE;
						//	//}

						//	s_nCheatZoneID = -1;
						//	return; // avoid parsing all events							
						//}

						s_nCheatZoneID = -1;
					}
					break;
				}
			} //switch (TouchEvents[3 * e] & EVENT_TYPE_MASK)
		} //if(cheatZoneID >= 0)
	} //for (int e = 0; e < TouchEventsNo; e++)
}

void CTouchZones::checkCheatZones(bool ingame)
{
	CHighGear *hg = CHighGear::GetInstance();

	if(ingame)
	{
		switch(s_nCheatID)
		{
			case k_CHEAT_GAME_COMPLETE:
			case k_CHEAT_WIN_RACE:
//			{
//				hg->m_PlayingGame->GetPlayerCar()->has_finished = true;
//				hg->m_PlayingGame->GetPlayerCar()->game_rank = 1;
//				hg->m_PlayingGame->m_WinPosition = 1;
//				hg->m_PlayingGame->SetRaceOver(1);
//				hg->m_PlayingGame->m_CheatQuickWin = 1;
//
//				switch( hg->m_options.type )
//				{
//					case RACE_DUEL:
//					case RACE_COP_CHASE:
//						hg->m_PlayingGame->m_WinPosition = 0;
//						break;
//
//					case RACE_BEAT_EM_ALL:
//						hg->m_PlayingGame->GetPlayerCar()->takedowns_count = BEATEMALL_TAKEDOWN_1;
//						break;
//
//					case RACE_CASH_ATTACK:
//						if (hg->m_PlayingGame->GetPlayerCar()->stats.nMoney < CASHATTACK_MONEY_1)
//							hg->m_PlayingGame->GetPlayerCar()->stats.nMoney = CASHATTACK_MONEY_1;
//						break;
//				}
//
//
//				if (s_nCheatID == k_CHEAT_GAME_COMPLETE)
//					hg->FinishAllRaces();
//			}	
				break;

			case k_CHEAT_LOOSE_RACE: 
//			{
//				hg->m_PlayingGame->GetPlayerCar()->game_rank = 8;
//				hg->m_PlayingGame->m_WinPosition = -1;
//				hg->m_PlayingGame->SetRaceOver(0);			
//			}
				break;
		}
		s_nCheatID = k_CHEAT_NONE;		
	}
	else
	{
		//only in main menu
		switch(s_nCheatID)
		{
			case k_CHEAT_UNLOCK_ALL:
//			{
//				hg->m_options.cheat_unlock_all = !hg->m_options.cheat_unlock_all;
//
//				setCheatName();
//			}
				break;
			
			case k_CHEAT_MONEY:
//			{
//				if( hg->m_Profile.unlock_idx < UNLOCK_PRICE_SIZE - 1 )
//				{					
//					hg->m_Profile.arcadeMoney = hg->m_unlockPrice[hg->m_Profile.unlock_idx + 1];
//					hg->m_Profile.arcadeMoneyCash = hg->m_unlockPrice[hg->m_Profile.unlock_idx + 1];
//					hg->m_Profile.arcadeMoneyTotal = hg->m_unlockPrice[hg->m_Profile.unlock_idx + 1];
//				}
//				else
//				{
//					hg->m_Profile.arcadeMoney += hg->m_unlockPrice[hg->m_Profile.unlock_idx];
//					hg->m_Profile.arcadeMoneyCash += hg->m_unlockPrice[hg->m_Profile.unlock_idx];
//					hg->m_Profile.arcadeMoneyTotal += hg->m_unlockPrice[hg->m_Profile.unlock_idx];
//				}
//
//				hg->LimitArcadeMoney();
//
//				hg->UpdateCompletionLevel();
//
//				MenuContainer::GetInstance()->updateCarsLockedStatus();
//
//				strcpy(s_cheatName, "!!!CHEAT_MONEY!!!");
//				s_showCheatCounter = k_CHEAT_VIEW_FRAMES;
//			}
				break;
					
		}


		s_nCheatID = k_CHEAT_NONE;
	}

}

void CTouchZones::drawActivatedCheat()
{
	CHighGear *hg = CHighGear::GetInstance();
//	if(
//		hg->m_state == CHighGear::gs_menu && 
//	   	MenuContainer::GetInstance()->getMenu() == MENU_MAIN_INDEX &&
//		s_showCheatCounter > 0
//	)
//	{
//		MenuContainer::GetInstance()->m_FontNormalWhite->DrawString(hg->GetLib2D(), hg->m_dispX >>1, 360, s_cheatName, CSprite::ALIGN_HCENTER_VCENTER);
//		s_showCheatCounter--;
//	}	
}

#endif //USE_CHEAT_ZONES

#endif // USE_TOUCH_SCREEN
