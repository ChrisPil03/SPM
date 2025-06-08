// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "DashComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "InteractInterface.h"
#include "GunBase.h"
#include "AbilitySystemComponent.h"
#include "CharacterMovementComponentAsync.h"
#include "DiveGameMode.h"
#include "PlayerAttributeSet.h"
#include "ScoreManagerComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	PlayerNearGroundThreshold = 150.0f;
	NodeMaxAerialGroundDistance = 200.0f;
	GroundCheckTraceDistance = 5000.0f;
	GroundTraceChannel = ECC_Visibility;
	bDrawMovementNodeDebugTraces = false;
	
	SolidObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	SolidObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->bUsePawnControlRotation = true;
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArmComponent->SetupAttachment(CameraComponent);
	
	GunComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Component"));
	GunComponent->SetupAttachment(SpringArmComponent);
	
	ScoreManagerComponent = CreateDefaultSubobject<UScoreManagerComponent>(TEXT("Score Manager Component"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<USphereComponent>(EnemyTargetSpheres);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UPlayerAttributeSet::GetHealthAttribute()
	).AddUObject(this, &APlayerCharacter::OnCurrentHealthChanged);
	
	InitPlayerStats();
	

	bDead = false;
	
	//GiveGun(Pistol);
	//GiveGun(Shotgun);
	//GiveGun(Rifle);
	//ChangeEquippedGun(2);
	
	UE_LOG(LogTemp, Display, TEXT("Begin play for character"));
	if (OnPlayerConstructed.IsBound())
	{
		OnPlayerConstructed.Broadcast();
	}
	
}

bool APlayerCharacter::IsPlayerConsideredNearGround(float& OutGroundDistance) const
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		OutGroundDistance = PlayerNearGroundThreshold + 1.f;
		return false;
	}

	if (MoveComp->IsMovingOnGround())
	{
		const FFindFloorResult& FloorResult = MoveComp->CurrentFloor;
		if (FloorResult.IsWalkableFloor())
		{
			OutGroundDistance = FloorResult.GetDistanceToFloor();
			return true;
		}
	}

	OutGroundDistance = PlayerNearGroundThreshold + 1.f;
	return false;
}

bool APlayerCharacter::IsNodeInsideSolid(const USphereComponent* Node) const
{
    if (!Node) return true;

    UWorld* World = GetWorld();
    if (!World) return true;

    TArray<AActor*> OverlappingActors;
    TArray<AActor*> ActorsToIgnore;
	
    bool bOverlapped = UKismetSystemLibrary::SphereOverlapActors(
        World,
        Node->GetComponentLocation(),
        Node->GetScaledSphereRadius(),
        SolidObjectTypes,
        nullptr,
        ActorsToIgnore,
        OverlappingActors
    );

    if (bDrawMovementNodeDebugTraces && bOverlapped && OverlappingActors.Num() > 0)
    {
        DrawDebugSphere(World, Node->GetComponentLocation(), Node->GetScaledSphereRadius() + 5.f, 12, FColor::Red, false, 1.0f, 0, 2.f);
    }
    else if (bDrawMovementNodeDebugTraces && !bOverlapped)
    {
         DrawDebugSphere(World, Node->GetComponentLocation(), Node->GetScaledSphereRadius() + 5.f, 12, FColor::Green, false, 1.0f, 0, 1.f);
    }


    return bOverlapped && OverlappingActors.Num() > 0;
}

bool APlayerCharacter::GetNodeDistanceToGround(const USphereComponent* Node, float& OutDistanceToGround) const
{
    if (!Node) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    FVector Start = Node->GetComponentLocation();
    FVector End = Start - FVector(0.f, 0.f, GroundCheckTraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (Node->GetOwner())
    {
        Params.AddIgnoredActor(Node->GetOwner());
    }


    bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, GroundTraceChannel, Params);

    if (bDrawMovementNodeDebugTraces)
    {
        DrawDebugLine(World, Start, bHit ? HitResult.ImpactPoint : End, bHit ? FColor::Blue : FColor::Orange, false, 1.0f, 0, 1.f);
    }

    if (bHit)
    {
        OutDistanceToGround = HitResult.Distance;
        return true;
    }

    OutDistanceToGround = GroundCheckTraceDistance + 1.f;
    return false;
}

TArray<USphereComponent*> APlayerCharacter::GetMovementNodes()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_PC_GetMovementNodes_Total"));

    TArray<TObjectPtr<USphereComponent>> TempValidMovementNodes;
    TempValidMovementNodes.Reserve(EnemyTargetSpheres.Num());

    float PlayerGroundDist;
    bool bPlayerIsNearGround;
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_PC_GetMovementNodes_IsPlayerNearGround"));
        bPlayerIsNearGround = IsPlayerConsideredNearGround(PlayerGroundDist);
    }

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_PC_GetMovementNodes_Loop"));
        for (TObjectPtr<USphereComponent> Node : EnemyTargetSpheres)
        {
            if (!Node) continue;

            // --- START: NEW LINE OF SIGHT CHECK ---
            bool bHasLineOfSight;
            {
                TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_PC_GetMovementNodes_LineOfSightCheck"));
                bHasLineOfSight = HasLineOfSightToNode(Node.Get());
            }
            if (!bHasLineOfSight)
            {
                continue;
            }
            // --- END: NEW LINE OF SIGHT CHECK ---
        
            bool bIsInsideSolid;
            {
                TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_PC_GetMovementNodes_IsNodeInsideSolid"));
                bIsInsideSolid = IsNodeInsideSolid(Node.Get());
            }
            if (bIsInsideSolid)
            {
                continue;
            }
        
            float NodeToGroundDist;
            bool bNodeHasGround;
            {
                TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("STAT_PC_GetMovementNodes_GetNodeDistToGround"));
                bNodeHasGround = GetNodeDistanceToGround(Node.Get(), NodeToGroundDist);
            }

            if (bPlayerIsNearGround)
            {
                if (!bNodeHasGround || NodeToGroundDist > NodeMaxAerialGroundDistance)
                {
                    continue;
                }
            }
            TempValidMovementNodes.Add(Node);
        }
    }

    TArray<USphereComponent*> ResultArray;
    ResultArray.Reserve(TempValidMovementNodes.Num());
    for (const TObjectPtr<USphereComponent>& NodePtr : TempValidMovementNodes)
    {
        ResultArray.Add(NodePtr.Get());
    }
    
	return ResultArray;
}

bool APlayerCharacter::HasLineOfSightToNode(const USphereComponent* Node) const
{
    if (!Node || !GetWorld())
    {
        return false;
    }

    FVector StartLocation = GetActorLocation();
    FVector EndLocation = Node->GetComponentLocation();

    FHitResult HitResult;
	
    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(LineOfSight), false, this);

    if (Node->GetOwner())
    {
        TraceParams.AddIgnoredActor(Node->GetOwner());
    }
	
    bool bHit = GetWorld()->LineTraceSingleByObjectType(
        HitResult,
        StartLocation,
        EndLocation,
        FCollisionObjectQueryParams(
            ECC_TO_BITFIELD(ECC_WorldStatic) |
            ECC_TO_BITFIELD(ECC_WorldDynamic) |
            ECC_TO_BITFIELD(ECC_GameTraceChannel2)), // Objective Channel
        TraceParams
    );
	
    return !bHit;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayerCharacter::PullTrigger()
{
	if (!EquippedGun)
	{
		return;
	}
	EquippedGun->StartFire();
}

void APlayerCharacter::ReleasedTrigger()
{
	if (!EquippedGun)
	{
		return;
	}
	EquippedGun->StopFire();
}

void APlayerCharacter::ActivateGunAbility()
{
	if (!EquippedGun)
	{
		return;
	}
	EquippedGun->ActivateAbility();
}

void APlayerCharacter::ReloadCurrentGun()
{
	if (!EquippedGun)
	{
		return;
	}
	ReleasedTrigger();
	EquippedGun->Reload();
}

void APlayerCharacter::Dash()
{
	UDashComponent *DashComponent = Cast<UDashComponent>(GetComponentByClass(UDashComponent::StaticClass()));
	if (DashComponent == nullptr)
	{
		return;
	}

	FVector Direction = GetVelocity();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}
	Direction = Direction.GetSafeNormal();

	DashComponent->Dash();
}


void APlayerCharacter::EquipWeapon(AGunBase* NewWeapon)
{
	if (EquippedGun)
	{
		EquippedGun->SetActorHiddenInGame(true);
		EquippedGun->CancelReload();
		// UE_LOG(LogTemp, Warning, TEXT("CancelReload"));
		ReleasedTrigger();
		EquippedGun->OnUnequipped();
	}

	EquippedGun = NewWeapon;

	if (EquippedGun)
	{
		EquippedGun->SetActorHiddenInGame(false);
		EquippedGun->OnEquipped();
	}
}


void APlayerCharacter::ChangeEquippedGun(int32 WeaponSlot)
{
	if (Guns.IsValidIndex(WeaponSlot) && Guns[WeaponSlot] != nullptr)
	{
		EquipWeapon(Guns[WeaponSlot]);
		OnWeaponChangedDelegate.Broadcast(WeaponSlot);
	}
	
	
}

void APlayerCharacter::Die()
{
	if (!IsDead())
	{
		bDead = true;
		OnPlayerDeathDelegate.Broadcast();
		ADiveGameMode *GameMode = GetWorld()->GetAuthGameMode<ADiveGameMode>();
		if (GameMode != nullptr)
		{
			GameMode->PlayerKilled(this);
			SaveScore();
		}
	}
}

bool APlayerCharacter::IsDead() const
{
	return bDead;
}

void APlayerCharacter::OnCurrentHealthChanged(const FOnAttributeChangeData& Data) const
{
	float NewCurrentHealth = Data.NewValue;
	OnCurrentHealthChangedDelegate.Broadcast(NewCurrentHealth);
}

void APlayerCharacter::GiveGun(const TSubclassOf<AGunBase> GunClass)
{
	EquippedGun = GetWorld()->SpawnActor<AGunBase>(GunClass);
	EquippedGun->AttachToComponent(GunComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	EquippedGun->SetOwner(this);
	EquippedGun->Initialize();
	EquippedGun->SetActorHiddenInGame(true);
	
	Guns.Add(EquippedGun);
}

void APlayerCharacter::InitPlayerStats()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(GE_InitPlayerStats, 1.f, Context);

		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Health"), Health);
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.MaxHealth"), MaxHealth);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			
		}
	}	

}

// ----------------- Interact ----------------- //
// -------------------------------------------- //

void APlayerCharacter::AddInteractable(IInteractInterface* Interactable)
{
	if (!AvailableInteractions.Contains(Interactable))
	{
		AvailableInteractions.Add(Interactable);	
	}
	if (Interactable->CanInteract())
	{
		ShowInteractPrompt(true);
	}
}

void APlayerCharacter::ClearInteractable(IInteractInterface* Interactable)
{
	if (AvailableInteractions.Contains(Interactable))
	{
		AvailableInteractions.Remove(Interactable);
	}
	for (IInteractInterface* Inter : AvailableInteractions)
	{
		if (Inter->CanInteract())
		{
			return;
		}
	}
	ShowInteractPrompt(false);
}

void APlayerCharacter::ShowInteractPrompt(const bool bShow) const
{
	if (!InteractWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interact widget not found"));
		return;
	}
	if (bShow && InteractWidget->GetVisibility() != ESlateVisibility::Visible)
	{
		InteractWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (!bShow && InteractWidget->GetVisibility() != ESlateVisibility::Collapsed)
	{
		InteractWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void APlayerCharacter::Interact()
{
	if (!AvailableInteractions.IsEmpty())
	{
		for (int i = AvailableInteractions.Num() - 1; i >= 0; i--)
		{
			if (AvailableInteractions[i]->CanInteract())
			{
				AvailableInteractions[i]->Interact(this);
				return;
			}
		}
	}
}
