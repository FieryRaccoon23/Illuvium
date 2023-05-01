// Fill out your copyright notice in the Description page of Project Settings.


#include "Cell.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACell::ACell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
	SetRootComponent(m_Mesh);
}

// Called when the game starts or when spawned
void ACell::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

