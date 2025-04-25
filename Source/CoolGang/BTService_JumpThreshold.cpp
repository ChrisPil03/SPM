// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_JumpThreshold.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAI.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_JumpThreshold::UBTService_JumpThreshold()
{
	NodeName = "Update Jump Threshold";
}
