﻿#include "SceneJungle.h"
#include "GL\glew.h"
#include "MeshBuilder.h"
#include "Application.h"
#include "LoadTGA.h"
#include <sstream>
#include "LevelLoader.h"

#include "Debug.h"

//Entity Includes
#include "Player.h"

#include "Coin.h"

//...

SceneJungle::SceneJungle()
{
	goManager = new GameObjectManager();
	inventory = new Inventory();
}

SceneJungle::~SceneJungle()
{
	input = NULL;
}

void SceneJungle::Init()
{
	SceneBase::Init();
	goManager->Init();

	// Calculating aspect ratio
	m_screenHeight = 100.f;
	m_screenWidth = m_screenHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	m_worldHeight = m_screenHeight * 2;
	m_worldWidth = m_screenWidth * 15;


	//Inventory init
	inventory->Init(this);

	//Physics code here
	m_speed = 1.f;
	Math::InitRNG();

	//Store keyboard instance
	input = Input::GetInstance();

	// Game Manager
	gameManager = GameManager::GetInstance();

	// Unique Meshes
	meshList[GEO_BG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1), 1.0f);
	meshList[GEO_BG]->textureID = LoadTGA("Image/bg_jungle2.tga");

	//Level Loading
	std::vector<GameObject*> tiles;
	if(LevelLoader::GetInstance()->LoadTiles("JUNGLE_1_1", this->meshList, this->tileSize, tiles, gridLength, gridHeight))
		DEBUG_MSG("Level Did not load successfully");
	for (auto& go : tiles)
	{
		if (go->geoTypeID == GEOMETRY_TYPE::GEO_PLAYER_GIRL1)
		{
			player = new Player();
			player->active = true;
			player->scale = go->scale;
			player->pos = go->pos;
			player->physics = go->physics->Clone();
			player->physics->SetInelasticity(0.99f);
			player->physics->SetIsBouncable(false);
			player->Init(&camera, Player::PLATFORMER, goManager, inventory);

			player->AddBottomSprite();
			player->bottomSprite->mesh = meshList[GEO_WALL];
			goManager->AddGO(player);


			DEBUG_MSG("From Phy Editor: " << player->scale);
			

			//Delete Grid Player
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_JUNGLE_MONKEY)
		{
			Monkey* monkey = new Monkey();

			monkey->active = true;
			monkey->scale = go->scale;
			monkey->pos = go->pos;
			monkey->physics = go->physics->Clone();
			monkey->physics->SetInelasticity(0.99f);
			monkey->physics->SetIsBouncable(false);
			monkey->physics->SetGravity(Vector3(0, 0, 0));
			monkey->Init(this, inventory, player->pos, new BulletSpawner(goManager, new BananaBullet(Vector3(2, 2, 2), monkey, 30)));

			monkey->AddBottomSprite();
			monkey->bottomSprite->mesh = meshList[GEO_WALL];
			goManager->AddGO(monkey);

			//Delete Grid monkey
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_COIN)
		{
			Coin* coin = new Coin(1);
			coin->active = true;
			coin->scale = go->scale * 0.85;
			coin->pos = go->pos;
			coin->physics = go->physics->Clone();
			coin->Init();
			goManager->AddGO(coin);

			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_JUNGLE_CAMPFIRE)
		{
			Campfire* campfire = new Campfire();

			campfire->active = true;
			campfire->scale = go->scale;
			campfire->pos = go->pos;
			campfire->physics = go->physics->Clone();
			campfire->physics->SetInelasticity(0.99f);
			campfire->physics->SetIsBouncable(false);
			campfire->physics->SetGravity(Vector3(0, 0, 0));
			campfire->Init(this, inventory, player->pos);

			goManager->AddGO(campfire);

			//Delete Grid campfire
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_JUNGLE_PROWLER)
		{
			Prowler* prowler = new Prowler();

			prowler->active = true;
			prowler->scale = go->scale;
			prowler->pos = go->pos;
			prowler->physics = go->physics->Clone();
			prowler->physics->SetInelasticity(0.99f);
			prowler->physics->SetIsBouncable(false);
			prowler->physics->SetGravity(Vector3(0, 0, 0));
			prowler->Init(this, inventory, player, goManager);

			goManager->AddGO(prowler);

			//Delete Grid campfire
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_COIN)
		{
			Coin* coin = new Coin(1);
			coin->active = true;
			coin->scale = go->scale * 0.85;
			coin->pos = go->pos;
			coin->physics = go->physics->Clone();
			coin->Init();
			goManager->AddGO(coin);
			delete go;
			go = nullptr;
		}
	}
	tiles.erase(std::remove(tiles.begin(), tiles.end(), nullptr), tiles.end());
	
	//Add all remainding tiles
	goManager->AddAllGO(tiles);
	
	//Camera init
	camera.Init(Vector3(0, 0, 1), Vector3(0, 0, 0), Vector3(0, 1, 0));
	camera.SetLimits(m_screenWidth, m_screenHeight, m_worldWidth, m_worldHeight);
	camera.SetFocusTarget(player->pos);
	camera.SetMode(Camera::CENTER);

	// ABILITIES
	/*DashAbility* ability = new DashAbility(meshList[GEO_ABILITYICON_DASH]);
	ability->SetCamera(&camera);
	ability->SetScenePointer(this);

	GrapplingAbility* ability2 = new GrapplingAbility(inventory, meshList[GEO_ABILITYICON_GRAPPLINGHOOK]);
	ability2->SetCamera(&camera);
	ability2->SetScenePointer(this);
	ability2->SetGOManager(this->goManager);*/

	gameManager->initAbilities(this, &camera, goManager, player);
	gameManager->setAbility(1, ABILITY_DASH);
	gameManager->setAbility(2, ABILITY_GRAPPLER);
	player->SetAbilities(gameManager->getCurrAbility(1), gameManager->getCurrAbility(2));
}

void SceneJungle::Update(double dt)
{
	SceneBase::Update(dt);
	inventory->Update(dt);
	camera.Update(player->pos, dt);

	if (input->IsKeyPressed('P'))
	{
		std::cout << "PRESSESD P" << std::endl;
		//Apple* newApple = new Apple();
		//inventory->AddItem(newApple);
		//inventory.setmax(i_apple, 10);
	}
	if (input->IsKeyPressed('O'))
	{
		std::cout << "PRESSESD O" << std::endl;
		//Cheese* newCheese = new Cheese();
		//inventory->AddItem(newCheese);
	}
	if (input->IsKeyPressed('L'))
	{
		std::cout << "PRESSED L" << std::endl;
		//Cheese* newCheese = new Cheese(2);
		//inventory->AddItem(newCheese);
	}

	if(input->IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(input->IsKeyPressed('0'))
	{
		m_speed += 0.1f;
	}

	goManager->Update(dt, &this->camera);

	if (player->currentHP <= 0)
	{
		gameLost = true;
	}
	if (gameManager->getMachineStatus(2))
	{
		gameWin = true;
	}
}

void SceneJungle::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
//	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projection.SetToOrtho(-1 * m_screenWidth * 0.5f, m_screenWidth * 0.5f, -1 * m_screenHeight * 0.5f, m_screenHeight * 0.5f, -10, 10);

	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	if (inventory->GetCurrentItem())
	{
		std::stringstream ss;
		ss << "curr item: " << inventory->GetCurrentItemType();

		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1.0f, 1.0f, 1.0f), 4, 10, 10);
	}

	// BG
	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y, -0.01);
	modelStack.Scale(m_screenWidth, m_screenHeight, 1);
	RenderMesh(meshList[GEO_BG], true);
	modelStack.PopMatrix();

	inventory->Render();
	goManager->Render(this);
}

void SceneJungle::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	goManager->Exit();
	inventory->Clear();

}
