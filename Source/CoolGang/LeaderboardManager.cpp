#include "LeaderboardManager.h"

/*
ULeaderboardManager::LeaderboardManager()
{
}

ULeaderboardManager::~LeaderboardManager()
{
}
*/

void ULeaderboardManager::AddEntry(const FString& PlayerName, int32 Score)
{
	Leaderboard.Add(FLeaderboardEntry(PlayerName, Score));
	SortLeaderboard();
}

TArray<FLeaderboardEntry> ULeaderboardManager::GetSortedLeaderboard() const
{
	return Leaderboard;
}

void ULeaderboardManager::SortLeaderboard()
{
	Leaderboard.Sort([](const FLeaderboardEntry& A, const FLeaderboardEntry& B)
	{
		return A.Score > B.Score; // Sort in descending order of scores
	});
}