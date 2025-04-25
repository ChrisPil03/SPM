#include "SystemIntegrity.h"

ASystemIntegrity::ASystemIntegrity()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASystemIntegrity::BeginPlay()
{
	Super::BeginPlay();
}

void ASystemIntegrity::WeakenIntegrity(const float Damage)
{
	CurrentIntegrity -= Damage;

	if (CurrentIntegrity <= 0)
	{
		SystemShutdown();
	}
}

void ASystemIntegrity::StrengthenIntegrity(const float Integrity)
{
	CurrentIntegrity += FMath::Clamp(Integrity, 0, MaxIntegrity);
}

void ASystemIntegrity::SystemShutdown()
{
	UE_LOG(LogTemp, Warning, TEXT("System shutdown"));
}

