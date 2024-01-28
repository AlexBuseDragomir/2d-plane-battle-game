#include "Bullet.h"

Bullet::Bullet(const BackBuffer *pBackBuffer, std::string owner)
{
	this->owner = owner;
	m_pSprite = new Sprite("data/bullet1.bmp", "data/bullet1_mask.bmp");
	m_pSprite->setBackBuffer(pBackBuffer);
}

Bullet::~Bullet()
{
	//bullet destructor code here
}

void Bullet::Move()
{
	// if the enemy shoots, the bullets will come from top -> bottom
	if (owner == "enemy")
	{
		this->m_pSprite->mPosition.y += 3;
	}

	// if the player shoots, the bullets will come from bottom -> top
	else
	{
		this->m_pSprite->mPosition.y -= 3;
	}	
}

void Bullet::Stop()
{
	this->m_pSprite->mPosition.y -= .001;
}
