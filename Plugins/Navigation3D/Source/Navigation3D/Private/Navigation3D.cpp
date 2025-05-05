// Copyright Epic Games, Inc. All Rights Reserved.

#include "Navigation3D.h" // Should match the module header file name

#define LOCTEXT_NAMESPACE "FNavigation3DModule"

void FNavigation3DModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// Add any initialization logic here
}

void FNavigation3DModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// Add any cleanup logic here
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FNavigation3DModule, Navigation3D) // Ensure 'Navigation3D' matches your module name