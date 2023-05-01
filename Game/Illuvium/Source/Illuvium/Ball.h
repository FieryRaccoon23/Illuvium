// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ball.generated.h"

UCLASS()
class ILLUVIUM_API ABall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* m_Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UPointLightComponent* m_Light;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* m_HealthIndicatorMesh;

	float m_LightIntensity = 1.0f;

	float m_LightIntensityMultiplier = 1000.0f;

	float m_HealthMeshScale = 1.0f;

	float m_HealthMeshScaleMultiplier = 0.5f;

private:
	float m_LightOffset = 120.0f;
};
