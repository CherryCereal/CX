#pragma once
#include "CXItemBase.h"
#include "CXCoinItem.generated.h"
UCLASS() class CX_API ACXCoinItem : public ACXItemBase { GENERATED_BODY() protected: virtual bool GrantReward(ACXPlayerState* PlayerState) override; };
