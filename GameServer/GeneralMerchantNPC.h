#pragma once
#include"NonPlayer.h"

class CGeneralMerchantNPC : public CNonPlayer
{
public:
	CGeneralMerchantNPC();
	~CGeneralMerchantNPC();

	void Update();

	void MerchantNPCInit();
private:
};

