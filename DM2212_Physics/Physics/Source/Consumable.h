#ifndef CONSUMABLE_H
#define CONSUMABLE_H
#include "Item.h"

class Consumable : public Item {
public:
	Consumable(ITEM_TYPE type);
	void Init() override;
	void Update(double dt) override;
	

private:
	
};


#endif