#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

UCLASS(BlueprintType)
class COOLGANG_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADoor();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Open();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Close();

	UFUNCTION(BlueprintPure)
	bool IsOpen() const { return bIsOpen; };

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bIsOpen;
};
