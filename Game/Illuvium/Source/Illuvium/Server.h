// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>

#include "Server.generated.h"

#define NUMBER_OF_PLAYERS 1
#define SERVER_FIXED_DELTA_TIME 0.1f

enum BallMovement
{
	Starting,
	Stationary,
	Moving,
	TargetReached
};

enum BallTeam
{
	Red,
	Blue
};

struct BallPosition
{
	int x = 0;
	int y = 0;

	BallPosition(){}
	BallPosition(int valueX, int valueY) : x(valueX), y (valueY){}

	// Usually this operator should not be a member but since the usage is not that much, I am making it a member
	bool operator== (BallPosition& rhs)
	{
		return ((x == rhs.x) && (y == rhs.y));
	}
};

class BallPlayer
{
	friend class CoreServer;

public:
	const BallPosition GetStartPosition() const { return m_StartPosition; }
	const BallPosition GetCurrentPosition() const { return m_CurrentPosition; }
	const float GetHealthPercentage() const { return (float)(m_HitPoints) / (float)(m_TotalHealth); }
	const bool IsStationary() const { return m_BallMovement == BallMovement::Stationary; }
	const bool IsMoving() const { return m_BallMovement == BallMovement::Moving; }
	const bool HasTargetReached() const { return m_BallMovement == BallMovement::TargetReached; }
	uint32 GetMoveFrame() const { return m_MoveFrame; }
	int GetPlayerId() const { return m_PlayerId; }
	BallTeam GetBallTeam() const { return m_Team; }

private:
	bool IsDead() const { return m_HitPoints == 0; }

	int m_PlayerId;
	BallTeam m_Team;
	int m_HitPoints = 0;
	int m_TotalHealth;
	BallPosition m_StartPosition;
	BallPosition m_CurrentPosition;
	std::vector<BallPosition> m_PathToDestination;
	bool m_IWillHitFirst;
	BallMovement m_BallMovement = BallMovement::Starting;
	int m_TargetEnemy;
	uint32 m_HitFrame = 0;
	uint32 m_MoveFrame = 0;
	int m_NextPositionIndex = 0;
	int m_LastPositionIndex = 0;
	int m_PathIndex = 0;
};

class GridCell
{
public:
	static const int m_DiagnolCost = 14;
	static const int m_StraightCost = 10;

	bool m_IsWalkable = true;
	bool m_Occupied = false;

	int m_gCost = -1;
	int m_hCost = -1;
	int m_fCost = -1;
	BallPosition m_ParentPosition;
};

class GridPath
{
	friend class Grid;

public:
	int GetRedPlayerId() const { return m_RedPlayerId; }
	int GetRedPlayerLocationIndex() const { return m_RedPlayerLocationIndex; }
	int GetBluePlayerId() const { return m_BluePlayerId; }
	int GetBluePlayerLocationIndex() const { return m_BluePlayerLocationIndex; }

	bool GetPathValid() const { return m_PathValid; }
	bool GetReached() const { return m_Reached; }

	int GetPathSize() const { return m_CommonPath.size(); }

	BallPosition GetPositionAt(int index) const 
	{ 
		if (index >= m_CommonPath.size())
		{
			index = m_CommonPath.size() - 1;
		}
		else if (index < 0)
		{
			index = 0;
		}

		return m_CommonPath[index];
	}

private:
	int m_RedPlayerId;
	int m_RedPlayerLocationIndex;

	int m_BluePlayerId;
	int m_BluePlayerLocationIndex;

	bool m_PathValid = true;
	bool m_Reached = false;

	std::vector<BallPosition> m_CommonPath;
};

class Grid
{
	friend class CoreServer;

public:
	int ManhattanDistance(BallPosition start, BallPosition end);
	int AStarPath(const BallPlayer& red, const BallPlayer& blue);

	static const int m_GridDimension = 10;
	GridCell m_Cells[m_GridDimension][m_GridDimension];

private:
	int UpdatePath(BallPlayer& player);
	std::vector<GridPath> m_Paths;

};

class CoreServer
{
	friend class AServer;

public:
	CoreServer(){}
	~CoreServer(){}

	const Grid* GetGrid() const { return &m_Grid; };
	int GetNumberOfPlayers() const { return NUMBER_OF_PLAYERS; }
	const BallPlayer* GetRedBallPlayers(int index) const { return index >= NUMBER_OF_PLAYERS ? nullptr : &m_RedBallPlayers[index]; }
	const BallPlayer* GetBlueBallPlayers(int index) const { return index >= NUMBER_OF_PLAYERS ? nullptr : &m_BlueBallPlayers[index]; }
	uint32 GetMoveTime() const { return m_MoveTime; }
	BallPosition GetBallNextPosition(const BallPlayer* player) const;
	BallPosition GetBallLastPosition(const BallPlayer* player) const;

private:
	void GenerateGrid();
	void InitRandomStream();
	void InitFrameData();
	int RandomIntFromStream(int min, int max);
	int RandomHitPoint();
	BallPosition RandomPosition();
	bool RandomBool();
	void GenerateRandomValuesForBalls();
	void UpdateState();
	void FindPath();
	bool CheckIfRedTeamDead();
	bool CheckIfBlueTeamDead();
	void Battle(BallPlayer& redPlayer, BallPlayer& bluePlayer);
	void PerformHit(BallPlayer& player, BallPlayer& target);
	BallPosition FindUnoccupiedLocationOnGrid(BallPosition position, int indexTillAdded);
	void FollowPath(BallPlayer& player);

	Grid m_Grid;
	FRandomStream m_RandomStream{};
	int m_RandomStreamSeed = 13579;
	bool m_HasStreamInit = false;
	BallPlayer m_RedBallPlayers[NUMBER_OF_PLAYERS];
	BallPlayer m_BlueBallPlayers[NUMBER_OF_PLAYERS];
	float m_TimeAccumulator = 0.0f;
	uint32 m_FrameNumber = 0;
	const uint32 m_AttackFrame = 2; // how many frames to attack. I made it 2 since, since 1 frame will be anticipation and the other will be hit.
	const uint32 m_MoveTime = 20; // move time in frames
	bool m_HasSimulationEnded = false;
};

UCLASS()
class ILLUVIUM_API AServer : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AServer();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Server Core functions
	static const CoreServer* GetCoreServer();
};
