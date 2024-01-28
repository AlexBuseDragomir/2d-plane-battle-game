#pragma once
#include "Main.h"
#include "Sprite.h"
#include "../Bullet.h"

class Enemy
{
public:
	Enemy(const BackBuffer *pBackBuffer);
	~Enemy();
	
	bool hit =              false;
	Sprite*					m_pSprite;
	int shootCooldown =     150;
	void move();
	void Shoot();
	bool left =             false;
};

