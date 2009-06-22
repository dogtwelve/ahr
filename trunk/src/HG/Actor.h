#ifndef __ACTOR_H__
#define __ACTOR_H__
#include "AuroraSprite.h"

class CActor
{
public:
	CActor(void);
	~CActor(void);

	enum
	{
		ACTOR_NONE = -1,
		ACTOR_MC,
		ACTOR_ENEMY1
	} ACTOR_TYPE;

	enum
	{
		ACTOR_IDLE = 0,
		ACTOR_ATTACK
	} ACTOR_STATE;


	void init(int, CSprite*);
	void draw(CLib2D);
	void update();

	int m_type;
	int m_state;
	void move(int);
private:
	
	int m_posX;
	int m_posY;
	
	int m_FrameCounter;
	int m_CurrentFrameTimer;
	int m_CurrentAFrame;
	int m_CurrentAnim;
	bool bStateChanged;
	CSprite* spr;
	void updateSprite();
	void setAnim(int);
};

#endif