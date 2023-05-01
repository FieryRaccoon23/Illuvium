// Fill out your copyright notice in the Description page of Project Settings.


#include "Client.h"
#include "Server.h"
#include "Ball.h"
#include "Cell.h"
#include "Wall.h"
#include "Floor.h"
#include "Engine/World.h"
#include <cmath>

// Sets default values
AClient::AClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AClient::BeginPlay()
{
	Super::BeginPlay();

	m_CachedNormalizedValue = 1.0f / (sqrtf(powf(1.0f, 2) + powf(1.0f, 2)));

	SpawnGrid();
	SpawnBalls();
}

// Called every frame
void AClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_TimeAccumulator += DeltaTime;

	while (m_TimeAccumulator >= CLIENT_FIXED_DELTA_TIME)
	{
		PerformFixedUpdate();
		m_TimeAccumulator -= CLIENT_FIXED_DELTA_TIME;
	}

	
}

void AClient::MoveRedBall(int index, float X, float Y)
{
	m_RedPlayers[index]->SetActorLocation(FVector(X, Y, 0.0f));
}

void AClient::MoveBlueBall(int index, float X, float Y)
{
	m_BluePlayers[index]->SetActorLocation(FVector(X, Y, 0.0f));
}

void AClient::LerpBall(const BallPlayer* ball, int index)
{
	BallPosition nextPos = AServer::GetCoreServer()->GetBallNextPosition(ball);
	BallPosition lastPos = AServer::GetCoreServer()->GetBallLastPosition(ball);

	float ratio = (float)(ball->GetMoveFrame()) / (float)(AServer::GetCoreServer()->GetMoveTime());

	float dX = (float)(nextPos.x - lastPos.x);
	float dY = (float)(nextPos.y - lastPos.y);

	if (abs(dX) == 1.0f && abs(dY) == 1.0f)
	{
		dX *= m_CachedNormalizedValue;
		dY *= m_CachedNormalizedValue;
	}

	dX *= m_FloorScale;
	dY *= m_FloorScale;
	dX *= ratio;
	dY *= ratio;

	float lastWorldPosX = (float)(lastPos.x) * m_FloorScale;
	float lastWorldPosY = (float)(lastPos.y)* m_FloorScale;

	if (ball->GetBallTeam() == BallTeam::Red)
	{
		MoveRedBall(index, m_Origin.X + lastWorldPosX + dX, m_Origin.Y + lastWorldPosY + dY);
	}
	else
	{
		MoveBlueBall(index, m_Origin.X + lastWorldPosX + dX, m_Origin.Y + lastWorldPosY + dY);
	}

}

void AClient::PerformFixedUpdate()
{
	// Update for visual cues for health
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		float redHealthPercent = AServer::GetCoreServer()->GetRedBallPlayers(i)->GetHealthPercentage();
		float blueHealthPercent = AServer::GetCoreServer()->GetBlueBallPlayers(i)->GetHealthPercentage();

		m_RedPlayers[i]->m_LightIntensity = redHealthPercent;
		m_BluePlayers[i]->m_LightIntensity = blueHealthPercent;

		m_RedPlayers[i]->m_HealthMeshScale = redHealthPercent;
		m_BluePlayers[i]->m_HealthMeshScale = blueHealthPercent;
	}

	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		const BallPlayer* red = AServer::GetCoreServer()->GetRedBallPlayers(i);
		if (red->IsMoving())
		{
			LerpBall(red, i);
		}

		const BallPlayer* blue = AServer::GetCoreServer()->GetBlueBallPlayers(i);
		if (blue->IsMoving())
		{
			LerpBall(blue, i);
		}
	}

}

void AClient::SpawnBalls()
{
	FTransform ballTransform;

	float floorScaleHalfSize = ((float)(m_FloorScale)) / 2.0f;
	m_Origin.X = floorScaleHalfSize;
	m_Origin.Y = floorScaleHalfSize;

	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		const BallPlayer* red = AServer::GetCoreServer()->GetRedBallPlayers(i);
		const BallPlayer* blue = AServer::GetCoreServer()->GetBlueBallPlayers(i);

		ballTransform.SetLocation(FVector(m_Origin.X + (red->GetStartPosition().x * m_FloorScale), m_Origin.Y + (red->GetStartPosition().y * m_FloorScale), 0.0f));
		ABall* redBall = GetWorld()->SpawnActor<ABall>(m_BPRedBall, ballTransform);
		m_RedPlayers.push_back(redBall);

		ballTransform.SetLocation(FVector(m_Origin.X + (blue->GetStartPosition().x * m_FloorScale), m_Origin.Y + +(blue->GetStartPosition().y * m_FloorScale), 0.0f));
		ABall* blueBall = GetWorld()->SpawnActor<ABall>(m_BPBlueBall, ballTransform);
		m_BluePlayers.push_back(blueBall);
	}
}

void AClient::SpawnGrid()
{
	int gridSize = AServer::GetCoreServer()->GetGrid()->m_GridDimension;

	//UE_LOG(LogTemp, Warning, TEXT("Grid size %d"), AServer::GetCoreServer()->GetGrid()->m_GridDimension);

	FTransform floorTransform;
	float gridHalfSize = ((float)(gridSize)) / 2.0f;
	float offset = (m_FloorScale) *gridHalfSize;
	floorTransform.SetLocation(FVector(offset, offset, 0.0f));
	floorTransform.SetScale3D(FVector(gridSize, gridSize, 1.0f));
	GetWorld()->SpawnActor<AFloor>(m_BPFloor, floorTransform);

	float x = 0.0f;
	float y = 0.0f;
	FTransform cellTransform;
	cellTransform.SetLocation(FVector(x, y, 0.0f));

	for (int i = 0; i < gridSize; ++i)
	{
		for (int j = 0; j < gridSize; ++j)
		{
			bool isWalkable = true;//AServer::GetCoreServer()->GetGrid()->GetIsWalkable(i, j);

			cellTransform.SetLocation(FVector(x, y, 0.0f));

			if (!isWalkable)
			{
				GetWorld()->SpawnActor<AWall>(m_BPWall, cellTransform);
			}

			x += m_FloorScale;
		}

		x = 0.0f;
		y += m_FloorScale;
	}
}