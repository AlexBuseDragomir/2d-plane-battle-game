//-----------------------------------------------------------------------------
// File: Bullet.cpp
//
// Desc: This file stores the bullet object class.
//
// Original design by Vasile Sebastian.
//-----------------------------------------------------------------------------

#ifndef _BULLET_H_
#define _BULLET_H_

#include <string>

#include "Main.h"
#include "Sprite.h"


class Bullet
{
public:
	Bullet(const BackBuffer *pBackBuffer, std::string owner = "");
	~Bullet();

	Sprite*					m_pSprite;

	std::string owner;
	void Move();
	void Stop();
};

#endif // !_BULLET_H_



