#include "CXSpeedBuffItem.h"
#include "CXPlayerState.h"
bool ACXSpeedBuffItem::GrantReward(ACXPlayerState* PlayerState){ return PlayerState->AddBonusMaxSpeed(10.f); }
