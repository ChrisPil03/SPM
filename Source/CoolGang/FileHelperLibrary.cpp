// Fill out your copyright notice in the Description page of Project Settings.


#include "FileHelperLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

TArray<FString> UFileHelperLibrary::GetAllFilesInDirectory(const FString& Directory, const FString& FileExtension)
{
	TArray<FString> FileNames;
	UE_LOG(LogTemp, Warning, TEXT("Looking for files! 11"));

	// Ensure the directory path is valid
	FString FullPath = FPaths::ConvertRelativePathToFull(Directory);
	if (!FPaths::DirectoryExists(FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Directory does not exist: %s"), *FullPath);
		return FileNames;
	}
	UE_LOG(LogTemp, Warning, TEXT("Looking for files!"));
	// Use IFileManager to find files
	IFileManager& FileManager = IFileManager::Get();
	FileManager.FindFiles(FileNames, *FString::Printf(TEXT("%s/%s"), *FullPath, *FileExtension), true, false);
	UE_LOG(LogTemp, Warning, TEXT("Found some files!"));
	return FileNames;
}