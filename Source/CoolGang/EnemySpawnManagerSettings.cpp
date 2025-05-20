// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawnManagerSettings.h"

UEnemySpawnManagerSettings::UEnemySpawnManagerSettings() // Constructor matches class name
{
	CategoryName = TEXT("Game"); // Or your custom category
	SectionName = TEXT("Enemy Spawn Manager Settings");
}

const UEnemySpawnManagerSettings* UEnemySpawnManagerSettings::Get()
{
	return GetDefault<UEnemySpawnManagerSettings>();
}