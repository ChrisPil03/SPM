// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FileHelperLibrary.generated.h"

/**
 * 
 */
UCLASS()
class COOLGANG_API UFileHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "FileHelper")
	static TArray<FString> GetAllFilesInDirectory(const FString& Directory, const FString& FileExtension);
};
