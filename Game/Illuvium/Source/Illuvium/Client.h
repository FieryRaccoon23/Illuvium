// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>

#include "Client.generated.h"

#define CLIENT_FIXED_DELTA_TIME 0.1f

class BallPlayer;

UCLASS()
class ILLUVIUM_API AClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AClient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Balls")
	TSubclassOf<class ABall> m_BPRedBall;

	UPROPERTY(EditAnywhere, Category = "Balls")
	TSubclassOf<class ABall> m_BPBlueBall;

	UPROPERTY(EditAnywhere, Category = "Walls")
	TSubclassOf<class AWall> m_BPWall;

	UPROPERTY(EditAnywhere, Category = "Floor")
	TSubclassOf<class AFloor> m_BPFloor;

private:
	void SpawnBalls();

	void SpawnGrid();

	void PerformFixedUpdate();

	void LerpBall(const BallPlayer* ball, int index);

	void MoveRedBall(int index, float X, float Y);

	void MoveBlueBall(int index, float X, float Y);

	const float m_FloorScale = 100.0f;

	FVector2D m_Origin;

	std::vector<class ABall*> m_RedPlayers;
	std::vector<class ABall*> m_BluePlayers;

	float m_TimeAccumulator = 0.0f;

	float m_CachedNormalizedValue;
};
