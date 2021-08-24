#ifndef ITEM_H
#define ITEM_H

#include "SceneBase.h"

class Item {
public:
	enum GROUP_TYPE {
		G_CONSUMABLE,
		G_WEAPON,
		G_TOTAL
	};

	enum ITEM_TYPE {
		I_CONSUMABLE,
		I_WEAPON,
		I_APPLE,
		I_CHEESE,
		I_PISTOL,
		I_FLASHLIGHT,
		I_BATTERY,
		I_FIRETORCH,
		I_GRAPPLER,
		I_BONE,
		I_SKULL,
		I_PICKAXE,
		I_TOTAL
	};

	Item();
	Item(GROUP_TYPE, ITEM_TYPE, int _quantity, bool _isStackable, Mesh* mesh);
	~Item();
	virtual void Init() = 0;
	virtual void Update(double dt) = 0;
	virtual bool IsEqual(Item* item) = 0;

	GROUP_TYPE GetGroupType();
	ITEM_TYPE GetType();

	int GetQuantity();
	void SetQuantity(int _quantity);
	int RemoveQuantity(int _quantity);

	float GetDurability();
	bool GetIsStackable();

	void SetScene(SceneBase* scene);

	Mesh* GetItemMesh();

protected:
	GROUP_TYPE groupType;
	ITEM_TYPE itemType;

	int quantity;
	float durability; // in percentage, 0 - none, 1 - max
	bool isStackable;

	SceneBase* scene;

	Mesh* mesh;
};


#endif