#include "Door.h"

ADoor::ADoor() :
	bIsOpen(false)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
}