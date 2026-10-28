#include "CXCoinItem.h"
#include "CXPlayerState.h"
bool ACXCoinItem::GrantReward(ACXPlayerState* PlayerState){ return PlayerState->AddCoins(10); }
