#ifndef __ACTOR_H__
#define __ACTOR_H__
#include "AuroraSprite.h"
#include "PlayingGame_defines.h"
#include "Random.h"

class CHighGear;


class CActor
{
public:
	CActor(CHighGear *pH);
	~CActor(void);

	enum
	{
		ACTOR_NONE = -1,
		ACTOR_MC,
		ACTOR_MUMMY,
		ACTOR_VAMPIRE,
		ACTOR_MCBULLET,
		ACTOR_BOOM
	} ACTOR_TYPE;

	enum
	{
		ACTOR_STATE_IDLE = 0,
		ACTOR_STATE_ATTACK,
		ACTOR_STATE_DAMAGED,
		ACTOR_STATE_DESTROYED
	} ACTOR_STATE;


	void init(int, CSprite*, int x = 0, int y = 0);
	void draw(CLib2D);
	void update();
	void notifyState(int state);

	int m_type;
	int m_state;
	void move(int);

	CHighGear* g_pGame;
	CSprite* spr;
	int m_posX;
	int m_posY;
private:	
	int m_VelocityCounter;
	int m_CurrentFrameTimer;
	int m_CurrentAFrame;
	int m_CurrentAnim;
	bool bStateChanged;
	void updateSprite();
	void setAnim(int);
};

#endif