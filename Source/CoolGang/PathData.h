// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PathData.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class COOLGANG_API UPathData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> Path;
};
