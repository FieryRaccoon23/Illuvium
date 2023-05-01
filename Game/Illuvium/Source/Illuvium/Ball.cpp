// Fill out your copyright notice in the Description page of Project Settings.


#include "Ball.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

// Sets default values
ABall::ABall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	m_Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	m_HealthIndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HealthIndicatorMesh"));;

	SetRootComponent(m_Mesh);
}

// Called when the game starts or when spawned
void ABall::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_Light->SetRelativeLocation(FVector(0.0f, 0.0f, m_LightOffset));
	m_Light->Intensity = m_LightIntensity * m_LightIntensityMultiplier;

	m_HealthIndicatorMesh->SetRelativeLocation(FVector(0.0f, 0.0f, m_LightOffset));
	float scaleValue = m_HealthMeshScaleMultiplier * m_HealthMeshScale;
	scaleValue = scaleValue <= 0.0f ? 0.001f : scaleValue;
	m_HealthIndicatorMesh->SetRelativeScale3D(FVector(scaleValue, scaleValue, scaleValue));
}

