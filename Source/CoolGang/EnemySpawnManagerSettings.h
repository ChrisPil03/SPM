// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EnemySpawnManagerSettings.generated.h"

	class UEnemySpawnDataTypes;

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Enemy Spawn Manager Settings"))
class COOLGANG_API UEnemySpawnManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UEnemySpawnManagerSettings();

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Configuration", meta=(AllowedClasses="/Script/CoolGang.EnemySpawnConfigurationDataAsset", DisplayName="Spawn Configuration Data Asset"))
	FSoftObjectPath SpawnConfigurationDataAssetPath;

	static const UEnemySpawnManagerSettings* Get();
};