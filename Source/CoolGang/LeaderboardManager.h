// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LeaderboardManager.generated.h"
/**
 * 
 */

USTRUCT(BlueprintType)
struct FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Leaderboard")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "Leaderboard")
	int32 Score;

	FLeaderboardEntry() : PlayerName(TEXT("")), Score(0) {}
	FLeaderboardEntry(FString Name, int32 PlayerScore) : PlayerName(Name), Score(PlayerScore) {}
};

UCLASS(Blueprintable)
class COOLGANG_API ULeaderboardManager : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void AddEntry(const FString& PlayerName, int32 Score);

	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	TArray<FLeaderboardEntry> GetSortedLeaderboard() const;

	/*
	ULeaderboardManager();
	~ULeaderboardManager();
	*/
private:
	TArray<FLeaderboardEntry> Leaderboard;

	void SortLeaderboard();
};
