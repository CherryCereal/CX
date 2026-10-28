#pragma once
#include "Commandlets/Commandlet.h"
#include "ConfigureHUDCommandlet.generated.h"
UCLASS()
class UConfigureHUDCommandlet : public UCommandlet
{
 GENERATED_BODY()
public:
 virtual int32 Main(const FString& Params) override;
};
