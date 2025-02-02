﻿#include "SceneRobot.h"
#include "GL\glew.h"
#include "MeshBuilder.h"
#include "Application.h"
#include "LoadTGA.h"
#include <sstream>
#include "LevelLoader.h"
#include "Utility.h"
#include "Debug.h"

//Entity Includes
#include "Player.h"
#include "PlasmaRobot.h"
#include "BlackHoleBullet.h"

#include "Coin.h"

//Ability Includes
#include "Recall.h"
#include "BlackHole.h"
#include "PlasmaEngine.h"

#include "Buttons/DialogueManager.h"

#include "SoundController/SoundController.h"

SceneRobot::SceneRobot()
{
	goManager = new GameObjectManager();
	inventory = new Inventory();
}

SceneRobot::~SceneRobot()
{
	input = NULL;
	if (goManager)
	{
		delete goManager;
		goManager = NULL;
	}
	if (inventory)
	{
		delete inventory;
		inventory = NULL;
	}
}

void SceneRobot::Init()
{
	SceneBase::Init();

	// Calculating aspect ratio
	m_screenHeight = 100.f;
	m_screenWidth = m_screenHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	m_worldHeight = 198                                          ;
	m_worldWidth = 1594;

	//Physics code here
	m_speed = 1.f;
	Math::InitRNG();

	// GO Manager
	goManager->Init();
	// Inventory 
	inventory->Init(this);

	//Store keyboard instance
	input = Input::GetInstance();
	// Game Manager
	gameManager = GameManager::GetInstance();

	// Unique Meshes
	meshList[GEO_BG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1), 1.0f);
	meshList[GEO_BG]->textureID = LoadTGA("Image/bg_robot.tga");

	playedDialogue = false;

	//Level Loading
	std::vector<GameObject*> tiles;
	if(LevelLoader::GetInstance()->LoadTiles("ROBOT_1_1", this->meshList, this->tileSize, tiles, gridLength, gridHeight))
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
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_ROBOT_ENEMY_1)
		{
			PlasmaRobot* robot = new PlasmaRobot();
			robot->active = true;
			robot->scale = go->scale;
			robot->pos = go->pos;
			robot->physics = go->physics->Clone();
			robot->Init(player, new BulletSpawner(goManager, new PlasmaBullet(Vector3(2, 2, 2), robot, 40)));
			robot->AddBottomSprite();
			robot->bottomSprite->mesh = meshList[GEO_WALL];

			goManager->AddGO(robot);
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
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_ROBOT_SMALLCUBE_9_MISCDECOR)
		{
			go->SetDamagableByExplosive(true);
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_ROBOT_SMALLCUBE_11_MISCDECOR)
		{
			go->SetDamagableByExplosive(true);
			go->SetRespawnable(true);
			
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_ROBOT_SMALLCUBE_16_MISCDECOR)
		{
			PlasmaEngine* engine = new PlasmaEngine();
			engine->active = true;
			GameObject::CloneValues(go, engine);
			engine->type = GameObject::GO_PLASMAENGINE;
			engine->AddToGOCollisionWhitelist(GameObject::GO_BULLET);

			PlasmaBullet* bullet = new PlasmaBullet(Vector3(2, 2, 2), engine, 40);
			bullet->SetExplosionRadius(15.5f);
			bullet->AddToGOCollisionWhitelist(GameObject::GO_PLASMAENGINE);
			engine->Init(
				new BulletSpawner(goManager, bullet),
				player, 7.0f);
			goManager->AddGO(engine);

			delete go;
			go = nullptr;
		}
	}
	tiles.erase(std::remove(tiles.begin(), tiles.end(), nullptr), tiles.end());
	
	// Add all remaining tiles
	goManager->AddAllGO(tiles);


	DEBUG_MSG("GOManager Robot: " << goManager);

	// Camera 
	camera.Init(Vector3(player->pos.x, player->pos.y, 1), Vector3(0, 0, 0), Vector3(0, 1, 0));
	camera.SetLimits(m_screenWidth, m_screenHeight, m_worldWidth, m_worldHeight);
	camera.SetFocusTarget(player->pos);
	camera.SetMode(Camera::CENTER);

	// ABILITIES
	gameManager->initAbilities(this, &camera, goManager, player);
	player->SetAbilities(gameManager->getCurrAbility(1), gameManager->getCurrAbility(2));

	DEBUG_MSG("Sound played");
	CSoundController::GetInstance()->PlaySoundByID(SOUND_TYPE::BG_ROBOTSCENE, 1.2, 0.4);
}

void SceneRobot::Update(double dt)
{
	SceneBase::Update(dt);
	//inventory->Update(dt);
	camera.Update(player->pos, dt);

	if (!playedDialogue)
	{
		playedDialogue = true;
		DialogueManager::GetInstance()->AddDialogue(PLAYER, "Robots!!...", PERSONA_DISPLAY::LEFT, 3.0f);
		DialogueManager::GetInstance()->AddDialogue(PLAYER, "another type II civilization,, this can't be good..", PERSONA_DISPLAY::LEFT, 3.0f);
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

	if (gameManager->getMachineStatus(1))
	{
		gameWin = true;
	}
	

}

void SceneRobot::Render()
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


	if (lights[0].type == Light::LIGHT_DIRECTIONAL)
	{
		Vector3 lightDir(lights[0].position.x, lights[0].position.y, lights[0].position.z);
		Vector3 lightDirection_cameraspace = viewStack.Top() * lightDir;
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, &lightDirection_cameraspace.x);
		//std::cout << "// DIRECTIONAL" << std::endl;
	}
	else if (lights[0].type == Light::LIGHT_SPOT)
	{
		Position lightPosition_cameraspace = viewStack.Top() * lights[0].position;
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, &lightPosition_cameraspace.x);
		Vector3 spotDirection_cameraspace = viewStack.Top() * lights[0].spotDirection;
		glUniform3fv(m_parameters[U_LIGHT0_SPOTDIRECTION], 1, &spotDirection_cameraspace.x);
	}
	else
	{
		Position lightPosition_cameraspace = viewStack.Top() * lights[0].position;
		glUniform3fv(m_parameters[U_LIGHT0_POSITION], 1, &lightPosition_cameraspace.x);
		//std::cout << "// POINT" << std::endl;
	}

	if (lights[1].type == Light::LIGHT_DIRECTIONAL)
	{
		Vector3 lightDir(lights[1].position.x, lights[1].position.y, lights[1].position.z);
		Vector3 lightDirection_cameraspace = viewStack.Top() * lightDir;
		glUniform3fv(m_parameters[U_LIGHT1_POSITION], 1, &lightDirection_cameraspace.x);
	}
	else if (lights[1].type == Light::LIGHT_SPOT)
	{
		Position lightPosition_cameraspace = viewStack.Top() * lights[1].position;
		glUniform3fv(m_parameters[U_LIGHT1_POSITION], 1, &lightPosition_cameraspace.x);
		Vector3 spotDirection_cameraspace = viewStack.Top() * lights[1].spotDirection;
		glUniform3fv(m_parameters[U_LIGHT1_SPOTDIRECTION], 1, &spotDirection_cameraspace.x);
	}
	else
	{
		Position lightPosition_cameraspace = viewStack.Top() * lights[1].position;
		glUniform3fv(m_parameters[U_LIGHT1_POSITION], 1, &lightPosition_cameraspace.x);
	}


	// BG
	modelStack.PushMatrix();
	modelStack.Translate(camera.position.x, camera.position.y, -0.01);
	modelStack.Scale(m_screenWidth, m_screenHeight, 1);
	RenderMesh(meshList[GEO_BG], true);
	modelStack.PopMatrix();

	goManager->Render(this);
}

void SceneRobot::InitLights()
{
	lights[0].type = Light::LIGHT_POINT;
	lights[0].position.Set(player->pos.x, player->pos.y, player->pos.z + 10);
	lights[0].color.Set(0.702, 0.529, 1);
	lights[0].power = 2;
	lights[0].defaultPower = lights[0].power;
	lights[0].kC = 1.f;
	lights[0].kL = 0.01f;
	lights[0].kQ = 0.001f;
	lights[0].cosCutoff = cos(Math::DegreeToRadian(45));
	lights[0].cosInner = cos(Math::DegreeToRadian(30));
	lights[0].exponent = 3.f;
	lights[0].spotDirection.Set(0.f, 0.f, 1.f);

	lights[1].type = Light::LIGHT_SPOT;
	lights[1].position.Set(0, 0, 1);
	lights[1].color.Set(0.8, 0.8, 1);
	lights[1].power = 0;
	lights[1].defaultPower = 2;
	lights[1].kC = 1.f;
	lights[1].kL = 0.01f;
	lights[1].kQ = 0.001f;
	lights[1].cosCutoff = cos(Math::DegreeToRadian(45));
	lights[1].cosInner = cos(Math::DegreeToRadian(30));
	lights[1].exponent = 3.f;
	lights[1].spotDirection.Set(0.f, 0.f, 1.f);

	// Make sure you pass uniform parameters after glUseProgram()
	glUniform1i(m_parameters[U_NUMLIGHTS], 2);

	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &lights[0].color.r);
	glUniform1f(m_parameters[U_LIGHT0_POWER], lights[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], lights[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], lights[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], lights[0].kQ);
	glUniform1i(m_parameters[U_LIGHT0_TYPE], lights[0].type);
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], lights[0].cosCutoff);
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], lights[0].cosInner);
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], lights[0].exponent);

	glUniform1i(m_parameters[U_LIGHT1_TYPE], lights[1].type);
	glUniform3fv(m_parameters[U_LIGHT1_COLOR], 1, &lights[1].color.r);
	glUniform1f(m_parameters[U_LIGHT1_POWER], lights[1].power);
	glUniform1f(m_parameters[U_LIGHT1_KC], lights[1].kC);
	glUniform1f(m_parameters[U_LIGHT1_KL], lights[1].kL);
	glUniform1f(m_parameters[U_LIGHT1_KQ], lights[1].kQ);
	glUniform1f(m_parameters[U_LIGHT1_COSCUTOFF], lights[1].cosCutoff);
	glUniform1f(m_parameters[U_LIGHT1_COSINNER], lights[1].cosInner);
	glUniform1f(m_parameters[U_LIGHT1_EXPONENT], lights[1].exponent);

	bLightEnabled = false;
}

void SceneRobot::CursorToWorldPosition(double& theX, double& theY)
{
	double x, y;
	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();
	// convert to world space
	x /= (w / m_screenWidth);
	y = h - y;
	y /= (h / m_screenHeight);
	x -= m_screenWidth * 0.5 - camera.position.x;
	y -= m_screenHeight * 0.5 - camera.position.y;

	theX = x;
	theY = y;
}

void SceneRobot::Exit()
{
	CSoundController::GetInstance()->StopPlayingSoundByID(SOUND_TYPE::BG_ROBOTSCENE);
	DEBUG_MSG("Sound stopped");
	SceneBase::Exit();
	//Cleanup GameObjects
	goManager->Exit();
	inventory->Clear();
}
