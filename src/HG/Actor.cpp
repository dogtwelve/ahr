#include "Actor.h"
#include "Lib2D.h"
#include "HighGear.h"

class CLib2D;
class CHighGear;

const int arrEnemyData[MAX_ENEMY][3]	=
{
	//speed, hp, score
	{ 10, 5, 0 },	//mummy
	{ 3, 3, 0, },	//vampire
	{ 7, 3, 0 }		//skull
};

#define ENEMY_DATA_INDEX_SPEED	0
#define ENEMY_DATA_INDEX_HP		1
#define ENEMY_DATA_INDEX_SCORE	2
CActor::CActor(CHighGear *p)
{
	m_type = ACTOR_NONE;
	g_pGame = p;
	spr = NULL;
}

CActor::~CActor(void)
{
}

void CActor::init(int type, CSprite* gameSpr, int x, int y, int level)
{
	m_type = type;
	spr = gameSpr;

	m_posX = x;
	m_posY = y;
	
	m_speed = arrEnemyData[m_type - ACTOR_MUMMY][ENEMY_DATA_INDEX_SPEED] + level / 2;
	m_hp = arrEnemyData[m_type - ACTOR_MUMMY][ENEMY_DATA_INDEX_HP] + level;
	m_score = arrEnemyData[m_type - ACTOR_MUMMY][ENEMY_DATA_INDEX_SCORE] + level / 5;

	m_VelocityCounter = 0;

	switch (type)
	{
	case ACTOR_ITEM:
		setAnim(0);
		m_state = ACTOR_STATE_IDLE;
		break;
	case ACTOR_MC:
	case ACTOR_MUMMY:
	case ACTOR_VAMPIRE:
	case ACTOR_SKULL:
		setAnim(0);
		m_state = ACTOR_STATE_IDLE;
		break;
	case ACTOR_MCBULLET:
		setAnim(1);
		m_state = ACTOR_STATE_IDLE;
		break;
	case ACTOR_BOOM:
		setAnim(0, false);
		m_state = ACTOR_STATE_IDLE;
		break;
	}
}


void CActor::notifyState(int state, int param1)
{
	if (m_type == ACTOR_NONE) return;

	m_state = state;
	switch (m_type)
	{
		case ACTOR_ITEM:
			m_state = ACTOR_STATE_DAMAGED;
			break;
		case ACTOR_MUMMY:
		case ACTOR_VAMPIRE:
		case ACTOR_SKULL:
			if (m_state == ACTOR_STATE_DAMAGED)
			{
				m_hp -= param1;
				if (m_hp <= 0)
				{
					m_state = ACTOR_STATE_DESTROYED;
					g_pGame->m_kill ++;
					//gen Items
					
					int index = g_pGame->getEmptyActorIndex();

					if (index > -1)
					{
						g_pGame->m_actors[index]->init(ACTOR_ITEM, g_pGame->GAMESPRITE_ITEM,
														m_posX, m_posY);
					}
					
				}
				else
				{
					if (m_posY > 0) m_posY --;
					m_state = ACTOR_STATE_IDLE;
				}
			}
			break;
		case ACTOR_MC:
			if (m_state == ACTOR_STATE_ATTACK)
			{
				setAnim(2, false);
				m_attDelay = param1;
			}
			else if (m_state == ACTOR_STATE_DAMAGED)
			{
				setAnim(1);
				m_VelocityCounter = GETTIMEMS();
			}
			break;
	}
}

void CActor::update()
{
	if (m_type == ACTOR_NONE) return;
	if (m_state == ACTOR_STATE_DESTROYED) return;
	switch (m_type)
	{
	case ACTOR_ITEM:
	{
		if (m_state == ACTOR_STATE_DAMAGED)
		{
			if (--m_posY <= 0)
				m_state = ACTOR_STATE_IDLE;
		}
		else
		{
			bool bHit = false;
			for (int i = 0; i < MAX_ACTOR; i ++)
			{
				if (g_pGame->m_actors[i] == NULL) continue;
				
				if (g_pGame->m_actors[i]->m_type != ACTOR_MCBULLET) continue;
				if (g_pGame->m_actors[i]->m_state == ACTOR_STATE_DESTROYED) continue;
				if (g_pGame->m_actors[i]->m_posX != m_posX) continue;
				if (g_pGame->m_actors[i]->m_posY != m_posY) continue;

				g_pGame->m_actors[i]->m_state = ACTOR_STATE_DESTROYED;	//Bullet X
					

				notifyState(ACTOR_STATE_DAMAGED, -1);	//ITEM
				//hit the Items!!!
				
				int index = g_pGame->getEmptyActorIndex();

				if (index > -1)
				{
					g_pGame->m_actors[index]->init(ACTOR_BOOM, g_pGame->GAMESPRITE_MCBULLET,
													m_posX, m_posY);
				}
				bHit = true;
				break;
			}

			if (!bHit 
				&& g_pGame->MAINCHAR->m_posX == m_posX &&
				g_pGame->MAINCHAR->m_posY == m_posY &&
				g_pGame->MAINCHAR->m_state != ACTOR_STATE_DAMAGED)
			{
				//get Items!
				m_state = ACTOR_STATE_DESTROYED;
				if (g_pGame->m_btDelay < MAX_BULLET_DELAY) g_pGame->m_btDelay++;
				if (g_pGame->m_btPow < MAX_BULLET_POWER) g_pGame->m_btPow++;
			}
			else if (++ m_posY > LEVEL_UNIT_HEIGHT)
			{
				m_state = ACTOR_STATE_DESTROYED;
			}
		}
		break;
	}
	case ACTOR_MC:
		if (m_state == ACTOR_STATE_ATTACK)
		{
			if (m_AnimLoopEnd == true)
				setAnim(0);
			if (-- m_attDelay <= 0) 
			{
				m_state = ACTOR_STATE_IDLE;
				
			}
		}
		else if (m_state == ACTOR_STATE_DAMAGED)
		{
			if (GETTIMEMS() - m_VelocityCounter > 500)
			{
				setAnim(0);
				m_state = ACTOR_STATE_IDLE;
			}
		}
		break;
	case ACTOR_SKULL:
	case ACTOR_MUMMY:
	case ACTOR_VAMPIRE:
		for (int i = 0; i < MAX_ACTOR; i ++)
		{
			if (g_pGame->m_actors[i] == NULL) continue;
			
			if (
				(	(g_pGame->m_actors[i]->m_type == ACTOR_MCBULLET && g_pGame->m_actors[i]->m_state != ACTOR_STATE_DESTROYED)
					|| 
					(g_pGame->m_actors[i]->m_type == ACTOR_MC && g_pGame->m_actors[i]->m_state != ACTOR_STATE_DAMAGED)
				) &&
				g_pGame->m_actors[i]->m_posX == m_posX && 
				g_pGame->m_actors[i]->m_posY == m_posY)
			{
				g_pGame->m_actors[i]->m_state = ACTOR_STATE_DESTROYED;	//Bullet X
				
				if (g_pGame->m_actors[i]->m_type == ACTOR_MCBULLET)
				{
					notifyState(ACTOR_STATE_DAMAGED, 1 + g_pGame->m_btPow);	//Enemy

					int index = g_pGame->getEmptyActorIndex();

					if (index > -1)
					{
						g_pGame->m_actors[index]->init(ACTOR_BOOM, g_pGame->GAMESPRITE_MCBULLET,
														m_posX, m_posY);
					}
				}
				else
				{
					g_pGame->m_actors[i]->notifyState(ACTOR_STATE_DAMAGED, -1);	//MC
				}
			}
		}
		if (m_state == ACTOR_STATE_IDLE)
		{
			//int v = arrEnemyData[m_type - ACTOR_MUMMY][ENEMY_DATA_INDEX_SPEED];
			if (++m_VelocityCounter > m_speed)
			{
				m_VelocityCounter = 0;

				if (++ m_posY >= LEVEL_UNIT_HEIGHT)
				{
					//Do some penalty to MC
					if (g_pGame->m_village > 0) g_pGame->m_village --;
					m_state = ACTOR_STATE_DESTROYED;
				}
			}
		}
		break;
	case ACTOR_MCBULLET:
		for (int i = 0; i < MAX_ACTOR; i ++)
		{
			if (g_pGame->m_actors[i] == NULL) continue;
			if (!g_pGame->m_actors[i]->isEnemy() 
				&& g_pGame->m_actors[i]->m_type != ACTOR_ITEM) continue;


			if (g_pGame->m_actors[i]->m_posX == m_posX && 
				g_pGame->m_actors[i]->m_posY == m_posY)
			{
				m_state = ACTOR_STATE_DESTROYED;	//Bullet X
				g_pGame->m_actors[i]->notifyState(ACTOR_STATE_DAMAGED, 1 + g_pGame->m_btPow);	//Enemy

				int index = g_pGame->getEmptyActorIndex();

				if (index > -1)
				{
					g_pGame->m_actors[index]->init(ACTOR_BOOM, g_pGame->GAMESPRITE_MCBULLET,
													g_pGame->m_actors[i]->m_posX,
													g_pGame->m_actors[i]->m_posY);
				}
			}
		}
		if (-- m_posY < 0)
			m_state = ACTOR_STATE_DESTROYED;
		break;
	case ACTOR_BOOM:
		if (m_AnimLoopEnd)
			m_state = ACTOR_STATE_DESTROYED;
	}
}

void CActor::setAnim(int anim, bool bLoop)
{
	m_CurrentAFrame = 0;
	m_CurrentFrameTimer = 0;
	m_CurrentAnim = anim;
	m_AnimLoop = bLoop;
	m_AnimLoopEnd = false;
}

void CActor::updateSprite()
{
	if (m_AnimLoopEnd) return;
	if (++m_CurrentFrameTimer > spr->GetAFrameTime(m_CurrentAnim, m_CurrentAFrame))
	{
		m_CurrentAFrame = (m_CurrentAFrame + 1) % spr->GetNumAFrames(m_CurrentAnim);

		if ( !m_AnimLoop && m_CurrentAFrame == 0)
		{
			m_AnimLoopEnd = true;
			m_CurrentAFrame = spr->GetNumAFrames(m_CurrentAnim) - 1;
		}
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

	if (this->isEnemy() && m_state != ACTOR_STATE_DESTROYED)
	{
		g.DrawRect(rX - (ENEMY_HP_BAR_WIDTH >> 1), rY + ENEMY_HP_BAR_OFFSET_Y, 
			ENEMY_HP_BAR_WIDTH, ENEMY_HP_BAR_HEIGHT,
			ENEMY_HP_BAR_COLOR_EMPTY, ENEMY_HP_BAR_COLOR_BORDER);

		if (m_hp > 0)
		{
			g.DrawRect(rX - (ENEMY_HP_BAR_WIDTH >> 1) + 1, rY + ENEMY_HP_BAR_OFFSET_Y + 1, 
				(ENEMY_HP_BAR_WIDTH - 2) * m_hp / 
					(arrEnemyData[m_type - ACTOR_MUMMY][ENEMY_DATA_INDEX_HP]+ level),
				ENEMY_HP_BAR_HEIGHT - 2,
				ENEMY_HP_BAR_COLOR, ENEMY_HP_BAR_COLOR);
		}
	}
}

void CActor::move(int _x)
{
	if (m_state != ACTOR_STATE_IDLE) return;

	
	m_posX += _x;
	if (m_posX < 0) m_posX = 0;
	if (m_posX > LEVEL_UNIT_WIDTH - 1) m_posX = LEVEL_UNIT_WIDTH - 1;
}

bool CActor::isEnemy()
{
	if (m_type == ACTOR_SKULL) return true;
	if (m_type == ACTOR_VAMPIRE) return true;
	if (m_type == ACTOR_MUMMY) return true;

	return false;
}

bool CActor::canFire()
{
	if (m_state == ACTOR_STATE_IDLE) return true;
	return false;
}