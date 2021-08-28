﻿#include "SceneLobby.h"
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

//...

SceneLobby::SceneLobby()
{
	dialogueManager = DialogueManager::GetInstance();
	//Store keyboard instance
	input = Input::GetInstance();
	goManager = new GameObjectManager();
	inventory = new Inventory();
	buttonManager = new ButtonManager(80, 60);

	machinePartsUIBG = MeshBuilder::GenerateQuad("Machine Parts Display BG", Color(1, 1, 1), 1.0f);
	machinePartsUIBG->textureID = LoadTGA("Image/TimeMachineBG.tga");
	machinePartsSlot = MeshBuilder::GenerateQuad("Machine Parts Slots", Color(1, 1, 1), 1.0f);
	machinePartsSlot->textureID = LoadTGA("Image/TimeMachinePartsSlot.tga");
	
}

SceneLobby::~SceneLobby()
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
	if (buttonManager)
	{
		delete buttonManager;
		buttonManager = NULL;
	}
	if (machinePartsUIBG)
	{
		delete machinePartsUIBG;
		machinePartsUIBG = NULL;
	}
	if (machinePartsSlot)
	{
		delete machinePartsSlot;
		machinePartsSlot = NULL;
	}
}

void SceneLobby::Init()
{
	SceneBase::Init();

	InitLights();

	// Calculating aspect ratio
	m_screenHeight = 100.f;

	//1920 x 1080
	m_screenWidth = m_screenHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	m_worldHeight = m_screenHeight;
	m_worldWidth = m_screenWidth;

	//Physics code here
	m_speed = 1.f;
	Math::InitRNG();

	// Game Manager
	gameManager = GameManager::GetInstance();
	// GO Manager
	goManager->Init();
	// Inventory 
	inventory->Init(this);
	
	// Unique Meshes
	meshList[GEO_BG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1), 1.0f);
	meshList[GEO_BG]->textureID = LoadTGA("Image/bg_lobby.tga");

	// Buttons
	if (gameManager->getMachineStatus(1))
	{
		Button* part = ButtonFactory::createNoTextButton("machinePart1", 31.25, 37.5,
			5, 5, meshList[GEO_MACHINEPART_1]);
		machinePartsUIButtons.push_back(part);
	}
	if (gameManager->getMachineStatus(2))
	{
		Button* part = ButtonFactory::createNoTextButton("machinePart2", 47.5, 37.5,
			5, 5, meshList[GEO_MACHINEPART_2]);
		machinePartsUIButtons.push_back(part);
	}
	if (gameManager->getMachineStatus(3))
	{
		Button* part = ButtonFactory::createNoTextButton("machinePart3", 31.25, 21.25,
			5, 5, meshList[GEO_MACHINEPART_3]);
		machinePartsUIButtons.push_back(part);
	}
	if (gameManager->getMachineStatus(4))
	{
		Button* part = ButtonFactory::createNoTextButton("machinePart4", 47.5, 21.25,
			5, 5, meshList[GEO_MACHINEPART_4]);
		machinePartsUIButtons.push_back(part);
	}
	Button* machinePartSlot1 = ButtonFactory::createNoTextButton("machinePartSlot1", 31.25, 37.5,
		15, 15, machinePartsSlot);
	machinePartsUIButtons.push_back(machinePartSlot1);
	Button* machinePartSlot2 = ButtonFactory::createNoTextButton("machinePartSlot2", 47.5, 37.5,
		15, 15, machinePartsSlot);
	machinePartsUIButtons.push_back(machinePartSlot2);
	Button* machinePartSlot3 = ButtonFactory::createNoTextButton("machinePartSlot3", 31.25, 21.25,
		15, 15, machinePartsSlot);
	machinePartsUIButtons.push_back(machinePartSlot3);
	Button* machinePartSlot4 = ButtonFactory::createNoTextButton("machinePartSlot4", 47.5, 21.25,
		15, 15, machinePartsSlot);
	machinePartsUIButtons.push_back(machinePartSlot4);

	Button* machinePartsBG = ButtonFactory::createNoTextButton("machinePartBG", 40, 30,
		40, 40, machinePartsUIBG);
	machinePartsUIButtons.push_back(machinePartsBG);

	for (int i = 0; i < machinePartsUIButtons.size(); i++)
	{
		machinePartsUIButtons[i]->disable();
		buttonManager->addButton(machinePartsUIButtons[i]);
	}
	
	showMachinePartsUI = false;

	//Level Loading
	std::vector<GameObject*> tiles;
	if(LevelLoader::GetInstance()->LoadTiles("LOBBY", this->meshList, this->tileSize, tiles, gridLength, gridHeight))
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

			player->Init(&camera, Player::WASD, goManager, inventory);

			player->AddBottomSprite();
			player->bottomSprite->mesh = meshList[GEO_WALL];
			goManager->AddGO(player, true);

			DEBUG_MSG("From Phy Editor: " << player->scale);


			//Delete Grid Player
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_LOBBY_PORTAL_GRAVEYARD)
		{
			portal_graveyard = new LobbyPortal();
			portal_graveyard->active = true;
			portal_graveyard->scale = go->scale;
			portal_graveyard->pos = go->pos;
			portal_graveyard->physics = go->physics->Clone();
			portal_graveyard->Init(red);
			goManager->AddGO(portal_graveyard);

			//Delete portal
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_LOBBY_PORTAL_JUNGLE)
		{
			portal_jungle = new LobbyPortal();
			portal_jungle->active = true;
			portal_jungle->scale = go->scale;
			portal_jungle->pos = go->pos;
			portal_jungle->physics = go->physics->Clone();
			portal_jungle->Init(green);
			goManager->AddGO(portal_jungle);

			//Delete portal
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_LOBBY_PORTAL_OCEAN)
		{
			portal_ocean = new LobbyPortal();
			portal_ocean->active = true;
			portal_ocean->scale = go->scale;
			portal_ocean->pos = go->pos;
			portal_ocean->physics = go->physics->Clone();
			portal_ocean->Init(blue);
			goManager->AddGO(portal_ocean);

			//Delete portal
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_LOBBY_PORTAL_ROBOT)
		{
			portal_robot = new LobbyPortal();
			portal_robot->active = true;
			portal_robot->scale = go->scale;
			portal_robot->pos = go->pos;
			portal_robot->physics = go->physics->Clone();
			portal_robot->Init(purple);
			goManager->AddGO(portal_robot);

			//Delete portal
			delete go;
			go = nullptr;
		}
		else if (go->geoTypeID == GEOMETRY_TYPE::GEO_LOBBY_MACHINE1)
		{
			timeMachine = go;
		}
	}
	tiles.erase(std::remove(tiles.begin(), tiles.end(), nullptr), tiles.end());

	// Add all remaining tiles
	goManager->AddAllGO(tiles);


	// Dialogue
	if (gameManager->getGameState() == GameManager::GS_INTRO)
	{
		dialogueManager->AddDialogue(PLAYER, "Ouch my head hurts..Where am I?");
		dialogueManager->AddDialogue(PLAYER, "What happened to me?");
		dialogueManager->AddDialogue(PLAYER, "Is this a time machine?");
		dialogueManager->AddDialogue(PLAYER, "??!!?!11!!?!");
		dialogueManager->AddDialogue(PLAYER, "(As you near the machine, )");
		gameManager->setGameState(GameManager::GS_DEFAULT);
	}

	// Camera 
	camera.Init(Vector3(m_screenWidth * 0.5, m_screenHeight * 0.5, 1), Vector3(m_screenWidth * 0.5, m_screenHeight * 0.5, 0), Vector3(0, 1, 0));
	camera.SetLimits(m_screenWidth, m_screenHeight, m_worldWidth, m_worldHeight);
	//camera.SetFocusTarget(player->pos);

	sceneManager = SceneManager::GetInstance();
}

void SceneLobby::Update(double dt)
{
	SceneBase::Update(dt);
	//inventory->Update(dt);
	camera.Update(player->pos, dt);

	// button manager
	buttonManager->Update(dt);
	// TIME MACHINE
	if (abs(timeMachine->pos.y - player->pos.y) < 20 && abs(timeMachine->pos.x - player->pos.x) < 10
		&& input->IsKeyPressed('E'))
	{
		showMachinePartsUI = !showMachinePartsUI;
		if (showMachinePartsUI)
		{
			for (int i = 0; i < machinePartsUIButtons.size(); i++)
			{
				buttonManager->activateButton(machinePartsUIButtons[i]->getName());
			}
		}
		else
		{
			for (int i = 0; i < machinePartsUIButtons.size(); i++)
			{
				buttonManager->deactivateButton(machinePartsUIButtons[i]->getName());
			}
		}
	}
	// PORTALS
	if (((portal_graveyard->pos.y - 10.5) == player->pos.y) && (player->pos.x >= (portal_graveyard->pos.x - 4)) && (player->pos.x <= (portal_graveyard->pos.x + 4)))
	{
		portal_graveyard->Open();
		if (Application::IsKeyPressed(VK_RETURN))
		{
			sceneManager->setScene(w_graveyard);
			CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		}
	}
	else {
		portal_graveyard->Close();
	}
	if (((portal_jungle->pos.y - 10.5) == player->pos.y) && (player->pos.x >= (portal_jungle->pos.x - 4)) && (player->pos.x <= (portal_jungle->pos.x + 4)))
	{
		portal_jungle->Open();
		if (Application::IsKeyPressed(VK_RETURN))
		{
			sceneManager->setScene(w_jungle);
			CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		}
	}
	else {
		portal_jungle->Close();
	}
	if (((portal_ocean->pos.y - 10.5) == player->pos.y) && (player->pos.x >= (portal_ocean->pos.x - 4)) && (player->pos.x <= (portal_ocean->pos.x + 4)))
	{
		portal_ocean->Open();
		if (Application::IsKeyPressed(VK_RETURN))
		{
			sceneManager->setScene(w_ocean);
			CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		}
	}
	else {
		portal_ocean->Close();
	}
	if (((portal_robot->pos.y - 10.5) == player->pos.y) && (player->pos.x >= (portal_robot->pos.x - 4)) && (player->pos.x <= (portal_robot->pos.x + 4)))
	{
		portal_robot->Open();
		if (Application::IsKeyPressed(VK_RETURN))
		{
			sceneManager->setScene(w_robot);
			CGameStateManager::GetInstance()->SetActiveGameState("PlayGameState");
		}
	}
	else {
		portal_robot->Close();
	}

	portal_graveyard->Update(dt);
	portal_jungle->Update(dt);
	portal_ocean->Update(dt);
	portal_robot->Update(dt);

	// DIALOGUE TEST
	if (input->IsKeyPressed('T'))
	{
		dialogueManager->AddDialogue(PLAYER, "TEST AAAAAAAAAAA");
	}
	if (input->IsKeyPressed('Y'))
	{
		dialogueManager->AddDialogue(GATEKEEPER, "LALALALALLALALALALALALALALALALALALALALALALALALALALAL", RIGHT);
	}

	if (Application::IsMousePressed(2))
	{

	}

	if(Application::IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed('0'))
	{
		m_speed += 0.1f;
	}

	if (!dialogueManager->Update(dt))
		goManager->Update(dt, &this->camera);
}

void SceneLobby::Render()
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
	modelStack.Translate(m_worldWidth * 0.5, m_worldHeight * 0.5, -0.01);
	modelStack.Scale(m_worldWidth, m_worldHeight, 1); 
	RenderMesh(meshList[GEO_BG], true);
	modelStack.PopMatrix();

	goManager->Render(this);
	buttonManager->Render(this);
	dialogueManager->Render(this);

	std::ostringstream ss;
	//ss.str("");
	//ss << "LIGHT COLOR: " << Vector3(lights[0].color.r, lights[0].color.g, lights[0].color.b);
	//RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3, 0, 6);
	/*ss.str("");
	ss << "player vel: " << player->physics->GetVelocity();
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3, 0, 9);*/
	/*ss.str("");
	ss << "camera pos: " << camera.position;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 0, 12);*/



	// fps tings
	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2, 0, 2);

	RenderTextOnScreen(meshList[GEO_TEXT], "Collision", Color(1, 1, 1), 2, 0, 0);
}

void SceneLobby::InitLights()
{
	lights[0].type = Light::LIGHT_POINT;
	lights[0].position.Set(0, 20, 0);
	lights[0].color.Set(1, 1, 1);
	lights[0].power = 4;
	lights[0].kC = 1.f;
	lights[0].kL = 0.01f;
	lights[0].kQ = 0.001f;
	lights[0].cosCutoff = cos(Math::DegreeToRadian(45));
	lights[0].cosInner = cos(Math::DegreeToRadian(30));
	lights[0].exponent = 3.f;
	lights[0].spotDirection.Set(0.f, 0.f, 1.f);

	lights[1].type = Light::LIGHT_SPOT;
	lights[1].position.Set(0, 0, 1);
	lights[1].color.Set(0, 0, 1);
	lights[1].power = 2;
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

void SceneLobby::CursorToWorldPosition(double& theX, double& theY)
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

void SceneLobby::Exit()
{
	SceneBase::Exit();
	goManager->Exit();
	inventory->Clear();
	for (int i = 0; i < machinePartsUIButtons.size(); i++)
	{
		buttonManager->deleteButton(machinePartsUIButtons[i]);
	}
	machinePartsUIButtons.clear();
}
