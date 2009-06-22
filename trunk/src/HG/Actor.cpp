#include "Actor.h"
#include "Lib2D.h"

class CLib2D;


CActor::CActor(void)
{
	m_type = ACTOR_NONE;
	spr = NULL;
}

CActor::~CActor(void)
{
}

#define MC_X_DEF 180
#define MC_Y_DEF 200

void CActor::init(int type, CSprite* gameSpr)
{
	m_type = type;
	switch (type)
	{
	case ACTOR_MC:
		spr = gameSpr;

		m_posX = MC_X_DEF;
		m_posY = MC_Y_DEF;
		//init sprite
		bStateChanged = false;
		setAnim(0);
		m_state = ACTOR_IDLE;
		break;
	}
}

void CActor::update()
{
	if (m_type == ACTOR_NONE)
		return;

	switch (m_type)
	{
	case ACTOR_MC:
		if (bStateChanged)
		{
			if (m_state == 0)
			{}
				
		}
		break;
	}
}

void CActor::setAnim(int anim)
{
	m_CurrentAFrame = 0;
	m_CurrentFrameTimer = 0;
	m_CurrentAnim = anim;
}

void CActor::updateSprite()
{
	if (++m_CurrentFrameTimer > spr->GetAFrameTime(m_CurrentAnim, m_CurrentAFrame))
	{
		m_CurrentAFrame = (m_CurrentAFrame + 1) % spr->GetNumAFrames(m_CurrentAnim);
	}
}

void CActor::draw(CLib2D g)
{
	if (m_type == ACTOR_NONE)
		return;
	if (spr != NULL)
		spr->DrawAFrame(g, m_posX, m_posY, m_CurrentAnim, m_CurrentAFrame);
	updateSprite();
}

void CActor::move(int _x)
{
	if (m_state != ACTOR_IDLE) return;

	m_posX += _x * 10;
}