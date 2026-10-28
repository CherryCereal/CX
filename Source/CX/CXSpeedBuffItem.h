#pragma once
#include "CXItemBase.h"
#include "CXSpeedBuffItem.generated.h"
UCLASS() class CX_API ACXSpeedBuffItem : public ACXItemBase { GENERATED_BODY() protected: virtual bool GrantReward(ACXPlayerState* PlayerState) override; };
