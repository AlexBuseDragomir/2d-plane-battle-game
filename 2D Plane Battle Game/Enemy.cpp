#include "Enemy.h"
#include "CGameApp.h"

// we need to use the instance of CGameApp that is defined in Main.cpp
// g_App -> Core game application processing engine
extern CGameApp g_App;

Enemy::Enemy(const BackBuffer *pBackBuffer)
{
	m_pSprite = new Sprite("data/enemy_plane.bmp", RGB(0xff, 0x00, 0xff));
	m_pSprite->setBackBuffer(pBackBuffer);
}

Enemy::~Enemy()
{
	//enemy destructor code here
}

void Enemy::move()
{	
	if (this->m_pSprite->mPosition.x == 200)
	{
		left = false;
	}

	if (this->m_pSprite->mPosition.x == 1700)
	{
		left = true;
	}
	
	if (this->m_pSprite->mPosition.x < 1700 && left == false)
	{
		this->m_pSprite->mPosition.x += 3;
	}

	if (this->m_pSprite->mPosition.x > 200 && left == true)
	{
		this->m_pSprite->mPosition.x -= 3;
	}
}

void Enemy::Shoot()
{
	if (shootCooldown < 5) {
		Bullet bullet(g_App.m_pBBuffer, "enemy");
		
		// enemy will shoot
		bullet.m_pSprite->mPosition = this->m_pSprite->mPosition;
		
		// save all instances of bullets in the container in order to draw them
		g_App.bulletsOnScreen.push_back(bullet);
		
		shootCooldown = 100;
	}
}
