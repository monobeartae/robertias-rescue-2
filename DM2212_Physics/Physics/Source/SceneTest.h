#ifndef SCENE_TEST_H
#define SCENE_TEST_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Player.h"
#include "GameObjectManager.h"

class SceneTest : public SceneBase
{
public:
	SceneTest();
	~SceneTest();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);

protected:


	//Physics
	std::vector<GameObject *> m_goList;
	
	Player* player;
	GameObject* testobj, *testWall;

	GameObjectManager* goManager;

	float m_speed;
	float m_screenWidth;
	float m_screenHeight;
	float m_worldWidth;
	float m_worldHeight;
	int m_objectCount;
	
	GameObject* FetchGO();
	GameObject* FetchGO(GameObject::GAMEOBJECT_TYPE type);
	GameObject* FetchGO(bool isMovable);
	void ReturnGO(GameObject *go);

	bool CheckZero(double number); // Checks if a number is 0 [EPSILON things]
	bool CheckCollision(GameObject* go1, GameObject* go2, float dt); // Checks for collision
	
	void Constraint(GameObject* ball, GameObject* other); // Moves ball out of other object upon collision

};

#endif