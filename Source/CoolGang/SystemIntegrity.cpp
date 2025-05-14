#include "SystemIntegrity.h"

#include "DiveGameMode.h"
#include "Kismet/GameplayStatics.h"

ASystemIntegrity::ASystemIntegrity() :
	MaxIntegrity(50000.f)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASystemIntegrity::BeginPlay()
{
	Super::BeginPlay();
	CurrentIntegrity = MaxIntegrity;
	bIsDestroyed = false;
}

void ASystemIntegrity::WeakenIntegrity(const float Damage)
{
	if (Damage < 0)
	{
		return;
	}
	CurrentIntegrity -= Damage;

	if (CurrentIntegrity <= 0 && !bIsDestroyed)
	{
		SystemShutdown();
	}
}

void ASystemIntegrity::StrengthenIntegrity(const float Integrity)
{
	if (Integrity < 0)
	{
		return;
	}
	if (CurrentIntegrity + Integrity < MaxIntegrity)
	{
		CurrentIntegrity += Integrity;
	}else
	{
		CurrentIntegrity = MaxIntegrity;
	}
}

void ASystemIntegrity::SystemShutdown()
{
	bIsDestroyed = true;
	Cast<ADiveGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->EndGame();
}

