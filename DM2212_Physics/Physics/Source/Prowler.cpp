
#include "Prowler.h"
#include "Application.h"
#include "MeshBuilder.h"
#include "LoadTGA.h"
#include "Flashlight.h"

Prowler::Prowler() : Enemy(JG_PROWLER)
{
}

Prowler::~Prowler()
{
}

void Prowler::Init(SceneBase* scene, Inventory* inventory, Vector3 &target, Weapon* _weapon)
{
	this->scene = scene;
	this->inventory = inventory;
	this->weapon = _weapon;
	playerPos = &target;

	state = IDLE;

	movement_speed = 20.0f;
	activeRange = 40.0f;
	shootTimer = 2;

	currentHP = 7;
	maxHP = 7; // IN SECONDS

	state_timer = 0;
	state_interval;

	physics->SetMovable(true);
	physics->SetEnableCollisionResponse(true);
	physics->SetGravity(Vector3(0,-98.f,0));

	animatedSprites = MeshBuilder::GenerateSpriteAnimation(20, 9, 2.0f, 2.0f);
	animatedSprites->AddAnimation("idleRight", 0, 4);
	animatedSprites->AddAnimation("idleLeft", 90, 94);
	animatedSprites->AddAnimation("runRight", 9, 16);
	animatedSprites->AddAnimation("runLeft", 99, 106);

	mesh = animatedSprites;
	mesh->textureID = LoadTGA("Image/prowler.tga");

	animatedSprites->PlayAnimation("idle", -1, 1.0f);
}

void Prowler::Update(double dt)
{ 
	mesh->material.kDiffuse.Set(1.0f, 1.0f, 1.0f);
	if (isBeingAttacked())
	{
		mesh->material.kDiffuse.Set(1.0f, 0.5f, 0.5f);
		state_interval = 0;
		state = ATTACK;
		currentHP -= dt;
	}

	if (currentHP <= 0)
	{
		dead = true;
		return;
	}
	switch (state)
	{
	case IDLE:
		if ((*playerPos - pos).Length() < activeRange)
		{
			state = JUMP;
		}
		break;
	case JUMP:
		physics->AddVelocity(Vector3((*playerPos - pos).Normalized().x * 5, 50, 0));
		state = ATTACK;
		break;
	case ATTACK:
	{
		if (!physics->GetOnGround())
			break;
		if (playerPos->y - 3 > pos.y)
		{
			state = JUMP;
			break;
		}
		else
		{
			shootTimer += dt;
			if (!(*playerPos - pos).IsZero())
			{
				physics->SetVelocity(Vector3((*playerPos - pos).Normalized().x * movement_speed, physics->GetVelocity().y, physics->GetVelocity().z));
				if (shootTimer >= 2)
				{
					weapon->GetBulletSpawner()->SpawnBullet(pos, Vector3((*playerPos - pos).Normalized().x, 0, 0), Vector3(0, 1, 0));
					shootTimer = 0;
				}
			}
		}
	}
	break;
	}

	animatedSprites->Update(dt);

	if (!physics->GetOnGround())
	{
		if (physics->GetVelocity().x > 0)
		{
			animatedSprites->PlayAnimation("jumpRight", -1, 1.0f);
		}
		else if (physics->GetVelocity().x < 0)
		{
			animatedSprites->PlayAnimation("jumpLeft", -1, 1.0f);
		}
	}
	else
	{
		if (physics->GetVelocity().x > 0)
		{
			animatedSprites->PlayAnimation("runRight", -1, 1.0f);
		}
		else if (physics->GetVelocity().x < 0)
		{
			animatedSprites->PlayAnimation("runLeft", -1, 1.0f);
		}
		else
		{
			animatedSprites->PlayAnimation("idle", -1, 1.0f);
		}
	}
}

void Prowler::StartAttackCooldown()
{
	state_interval = 2.0f;
}

bool Prowler::isBeingAttacked()
{
	/*Flashlight* flashlight = dynamic_cast<Flashlight*>(inventory->GetCurrentItem());
	if (flashlight->isIntensified())
	{
		return true;
	}*/
	return false;
}