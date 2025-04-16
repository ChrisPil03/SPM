// Fill out your copyright notice in the Description page of Project Settings.


#include "SphereTriggerComponent.h"
#include "ObjectiveDownloadStation.h"

USphereTriggerComponent::USphereTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Objective = Cast<AObjectiveDownloadStation>(GetOwner());
}

void USphereTriggerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USphereTriggerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Actor = GetAcceptableActor();
	
	if (Objective)
	{
		if (Actor)
		{
			Objective->ResumeObjective();
		}else
		{
			//UE_LOG(LogTemp, Display, TEXT("Pause"));
			Objective->AbortObjective();
		}
	}
}

AActor* USphereTriggerComponent::GetAcceptableActor() const
{
	TArray<AActor*> Actors;
	GetOverlappingActors(Actors);
	for (AActor* Actor : Actors)
	{
		if (Actor->ActorHasTag(TagToCheckFor))
		{
			//UE_LOG(LogTemp, Display, TEXT("Processing objective: %s"), *Actor->GetName());
			return Actor;
		}
	}
	return nullptr;
}
