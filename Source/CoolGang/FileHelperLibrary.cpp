// Fill out your copyright notice in the Description page of Project Settings.


#include "FileHelperLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

TArray<FString> UFileHelperLibrary::GetAllFilesInDirectory(const FString& Directory, const FString& FileExtension)
{
	TArray<FString> FileNames;
	UE_LOG(LogTemp, Warning, TEXT("Looking for files!"));

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

	// Print the number of files found
	UE_LOG(LogTemp, Warning, TEXT("Found %d files."), FileNames.Num());

	// Prepare array for stripped names
	TArray<FString> StrippedFileNames;

	for (const FString& FileName : FileNames)
	{
		UE_LOG(LogTemp, Warning, TEXT("File: %s"), *FileName);

		// If the file name ends with ".sav", remove it
		if (FileName.EndsWith(TEXT(".sav")))
		{
			StrippedFileNames.Add(FileName.LeftChop(4));
		}
		else
		{
			StrippedFileNames.Add(FileName);
		}
	}

	// Optionally print the stripped names
	for (const FString& Stripped : StrippedFileNames)
	{
		UE_LOG(LogTemp, Warning, TEXT("Stripped File Name: %s"), *Stripped);
	}

	return StrippedFileNames;
}