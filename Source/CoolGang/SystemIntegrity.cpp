#include "SystemIntegrity.h"

ASystemIntegrity::ASystemIntegrity() :
	MaxIntegrity(100000.f), CurrentIntegrity(MaxIntegrity)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASystemIntegrity::BeginPlay()
{
	Super::BeginPlay();
}

void ASystemIntegrity::WeakenIntegrity(const float Damage)
{
	if (Damage < 0)
	{
		return;
	}
	CurrentIntegrity -= Damage;

	if (CurrentIntegrity <= 0)
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
	UE_LOG(LogTemp, Warning, TEXT("System shutdown"));
}

