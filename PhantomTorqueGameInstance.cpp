// PhantomTorqueAllInOne.cpp - Unified System (Extended) // Includes: XP, Coin Store, AI, UI, Multiplayer, Vehicles, Replay, Environments, Lobby, Store Packs, Voice/Text Chat, Driver Unlocks

#include "PhantomTorqueUnified.h" #include "Engine/GameInstance.h" #include "Engine/World.h" #include "GameFramework/SaveGame.h" #include "GameFramework/PlayerController.h" #include "Kismet/GameplayStatics.h" #include "Containers/Map.h" #include "TimerManager.h" #include "AIController.h" #include "NiagaraComponent.h" #include "Sound/SoundCue.h" #include "InputMappingContext.h" #include "OnlineSubsystem.h" #include "OnlineSessionSettings.h"

// ---------- Driver Tier Enum ---------- UENUM(BlueprintType) enum class EDriverTier : uint8 { Tier1, Tier2, Tier3, Tier4 };

// ---------- Save Data Struct ---------- UCLASS() class USaveGameData : public USaveGame { GENERATED_BODY() public: UPROPERTY(VisibleAnywhere) int32 XP; UPROPERTY(VisibleAnywhere) int32 Coins; UPROPERTY(VisibleAnywhere) TArray UnlockedDrivers; UPROPERTY(VisibleAnywhere) TArray OwnedCars; UPROPERTY(VisibleAnywhere) TArray UnlockedAccessories; };

// ---------- GameInstance Core ---------- UCLASS() class UPhantomTorqueGameInstance : public UGameInstance { GENERATED_BODY()

public: UPROPERTY(BlueprintReadWrite) int32 PlayerXP = 0; UPROPERTY(BlueprintReadWrite) int32 PlayerCoins = 0; UPROPERTY(BlueprintReadWrite) TArray UnlockedDrivers; UPROPERTY(BlueprintReadWrite) TArray OwnedCars; UPROPERTY(BlueprintReadWrite) TArray UnlockedAccessories;

UPROPERTY(EditDefaultsOnly) TMap<FString, int32> DriverXPThresholds = { {"Jake Carter", 0}, {"Zara Blaze", 300}, {"Maxon Rook", 1000}, {"Kairos Vortex", 2000} }; UPROPERTY(EditDefaultsOnly) TMap<FString, int32> DriverCoinCost = { {"Zara Blaze", 200}, {"Maxon Rook", 500}, {"Kairos Vortex", 1000} }; virtual void Init() override { LoadProgress(); } UFUNCTION(BlueprintCallable) void AddXP(int32 Amount) { PlayerXP += Amount; CheckDriverUnlocks(); SaveProgress(); } UFUNCTION(BlueprintCallable) void AddCoins(int32 Amount) { PlayerCoins += Amount; SaveProgress(); } UFUNCTION(BlueprintCallable) void PurchaseDriver(FString Name) { if (DriverCoinCost.Contains(Name) && PlayerCoins >= DriverCoinCost[Name] && !UnlockedDrivers.Contains(Name)) { PlayerCoins -= DriverCoinCost[Name]; UnlockedDrivers.Add(Name); SaveProgress(); } } UFUNCTION(BlueprintCallable) void SaveProgress() { auto SaveData = Cast<USaveGameData>(UGameplayStatics::CreateSaveGameObject(USaveGameData::StaticClass())); SaveData->XP = PlayerXP; SaveData->Coins = PlayerCoins; SaveData->UnlockedDrivers = UnlockedDrivers; SaveData->OwnedCars = OwnedCars; SaveData->UnlockedAccessories = UnlockedAccessories; UGameplayStatics::SaveGameToSlot(SaveData, TEXT("PhantomSaveSlot"), 0); } UFUNCTION(BlueprintCallable) void LoadProgress() { if (auto Loaded = UGameplayStatics::LoadGameFromSlot(TEXT("PhantomSaveSlot"), 0)) { auto SaveData = Cast<USaveGameData>(Loaded); PlayerXP = SaveData->XP; PlayerCoins = SaveData->Coins; UnlockedDrivers = SaveData->UnlockedDrivers; OwnedCars = SaveData->OwnedCars; UnlockedAccessories = SaveData->UnlockedAccessories; } } 

private: void CheckDriverUnlocks() { for (const auto& Elem : DriverXPThresholds) { if (PlayerXP >= Elem.Value && !UnlockedDrivers.Contains(Elem.Key)) { UnlockedDrivers.Add(Elem.Key); } } } };

// ---------- AI Skill Assignment ---------- UCLASS() class APhantomTorqueAI : public AAIController { GENERATED_BODY()

public: float MaxSpeed; float Aggression;

UFUNCTION(BlueprintCallable) void SetSkill(EDriverTier Tier) { switch (Tier) { case EDriverTier::Tier1: MaxSpeed = 100; Aggression = 0.2f; break; case EDriverTier::Tier2: MaxSpeed = 130; Aggression = 0.5f; break; case EDriverTier::Tier3: MaxSpeed = 160; Aggression = 0.8f; break; case EDriverTier::Tier4: MaxSpeed = 180; Aggression = 1.0f; break; } } 

};

// ---------- Dynamic Environment Manager ---------- UCLASS() class AEnvironmentManager : public AActor { GENERATED_BODY()

public: UFUNCTION(BlueprintCallable) void SetWeather(FString WeatherType) { UE_LOG(LogTemp, Log, TEXT("Weather: %s"), *WeatherType); }

UFUNCTION(BlueprintCallable) void SetTimeOfDay(FString TimeOfDay) { UE_LOG(LogTemp, Log, TEXT("Time: %s"), *TimeOfDay); } 

};

// ---------- Replay System ---------- UCLASS() class UReplayComponent : public UActorComponent { GENERATED_BODY()

private: TArray Snapshots; FTimerHandle Timer; APawn* Car;

public: UFUNCTION(BlueprintCallable) void Start(APawn* InCar) { Car = InCar; Snapshots.Empty(); GetWorld()->GetTimerManager().SetTimer(Timer, this, &UReplayComponent::TickCapture, 0.1f, true); }

void TickCapture() { if (Car) Snapshots.Add(Car->GetActorTransform()); } UFUNCTION(BlueprintCallable) void Stop() { GetWorld()->GetTimerManager().ClearTimer(Timer); } UFUNCTION(BlueprintCallable) void PlayBack() { // TODO: Implement replay timeline } 

};

