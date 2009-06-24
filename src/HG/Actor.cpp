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

void CActor::init(int type, CSprite* gameSpr, int x, int y)
{
	m_type = type;
	switch (type)
	{
	case ACTOR_MC:
	case ACTOR_ENEMY1:
		spr = gameSpr;

		m_posX = x;
		m_posY = y;
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
	case ACTOR_ENEMY1:
		if (m_state == ACTOR_IDLE)
		{
			m_posY = (m_posY + 1) % 6;
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
	if (m_type == ACTOR_NONE) return;
	if (spr == NULL) return;

	//get current w
	int rY = LEVEL_Y_START + LEVEL_PIXEL_HEIGHT * m_posY / (LEVEL_UNIT_HEIGHT - 1);
	int tempW = LEVEL_PIXEL_WIDTH_SHORT + (LEVEL_PIXEL_WIDTH_LONG - LEVEL_PIXEL_WIDTH_SHORT) * m_posY / (LEVEL_UNIT_HEIGHT - 1);
	int rX = LEVEL_X_CENTER - (tempW >> 1) + m_posX * tempW / (LEVEL_UNIT_WIDTH-1);
	spr->DrawAFrame(g, rX, rY, m_CurrentAnim, m_CurrentAFrame);
	updateSprite();
}

void CActor::move(int _x)
{
	if (m_state != ACTOR_IDLE) return;

	
	m_posX += _x;
	if (m_posX < 0) m_posX = 0;
	if (m_posX > LEVEL_UNIT_WIDTH - 1) m_posX = LEVEL_UNIT_WIDTH - 1;
}