
#include "Enemy.h"
#include "Application.h"
#include "MeshBuilder.h"
#include "LoadTGA.h"

Enemy::Enemy(const ENEMY_TYPE type)
	: enemyType(type)
	, animatedSprites(nullptr)
{
	this->type = GO_ENEMY;
}

void Enemy::Init()
{
	physics->SetMovable(true);
	physics->SetIsBouncable(false);
	physics->SetInelasticity(0.99f);
}

void Enemy::Update(double dt)
{ 
	
}

const ENEMY_TYPE Enemy::GetEnemyType()
{
	return enemyType;
}
