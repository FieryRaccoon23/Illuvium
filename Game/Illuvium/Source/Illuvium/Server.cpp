// Fill out your copyright notice in the Description page of Project Settings.


#include "Server.h"

static CoreServer s_CoreServer = {};

const CoreServer* AServer::GetCoreServer()
{
	return &s_CoreServer;
}

// Sets default values
AServer::AServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AServer::BeginPlay()
{
	Super::BeginPlay();
	
	// Init Core Server
	s_CoreServer.GenerateGrid();
	s_CoreServer.InitRandomStream();
	s_CoreServer.GenerateRandomValuesForBalls();
	s_CoreServer.InitFrameData();
	s_CoreServer.FindPath();
}

// Called every frame
void AServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	s_CoreServer.m_TimeAccumulator += DeltaTime;

	while (s_CoreServer.m_TimeAccumulator >= SERVER_FIXED_DELTA_TIME)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Delta time %f"), s_CoreServer.m_TimeAccumulator);
		s_CoreServer.UpdateState();
		s_CoreServer.m_TimeAccumulator -= SERVER_FIXED_DELTA_TIME;
	}
}

/////////////////////CORE SERVER/////////////////////
void CoreServer::GenerateGrid()
{
	//UE_LOG(LogTemp, Warning, TEXT("Start grid generation ..."));

}

void CoreServer::InitRandomStream()
{
	m_RandomStream.Initialize(m_RandomStreamSeed);
	m_HasStreamInit = true;
}

void CoreServer::InitFrameData()
{
	m_TimeAccumulator = 0.0f;
	m_FrameNumber = 0;
}

int CoreServer::RandomIntFromStream(int min, int max)
{
	if (!m_HasStreamInit)
	{
		UE_LOG(LogTemp, Error, TEXT("Random Stream not initialized!"));
	}

	 return m_RandomStream.RandRange(min, max);
}

int CoreServer::RandomHitPoint()
{
	return RandomIntFromStream(2, 5);
}

BallPosition CoreServer::RandomPosition()
{
	BallPosition position;

	position.x = RandomIntFromStream(0, Grid::m_GridDimension - 1);
	position.y = RandomIntFromStream(0, Grid::m_GridDimension - 1);

	return position;
}

bool CoreServer::RandomBool()
{
	return RandomIntFromStream(0, 1) == 1;
}

void CoreServer::GenerateRandomValuesForBalls()
{
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		BallPlayer& redPlayer = m_RedBallPlayers[i];
		redPlayer.m_PlayerId = i;
		redPlayer.m_StartPosition = FindUnoccupiedLocationOnGrid(RandomPosition(), i);
		redPlayer.m_CurrentPosition = redPlayer.m_StartPosition;
		redPlayer.m_HitPoints = RandomHitPoint();
		redPlayer.m_TotalHealth = redPlayer.m_HitPoints;
		redPlayer.m_Team = BallTeam::Red;

		BallPlayer& bluePlayer = m_BlueBallPlayers[i];
		bluePlayer.m_StartPosition = FindUnoccupiedLocationOnGrid(RandomPosition(), i);
		bluePlayer.m_CurrentPosition = bluePlayer.m_StartPosition;
		bluePlayer.m_HitPoints = RandomHitPoint();
		bluePlayer.m_TotalHealth = bluePlayer.m_HitPoints;
		bluePlayer.m_PlayerId = i;
		bluePlayer.m_Team = BallTeam::Blue;
	}
}

BallPosition CoreServer::FindUnoccupiedLocationOnGrid(BallPosition position, int indexTillAdded)
{
	bool result = true;

	for (int i = 0; i < indexTillAdded; ++i)
	{
		if (m_RedBallPlayers[i].m_StartPosition == position || m_BlueBallPlayers[i].m_StartPosition == position)
		{
			result = false;
		}
	}

	if (result && !m_Grid.m_Cells[position.x][position.y].m_IsWalkable)
	{
		result = false;
	}

	if (result)
	{
		return position;
	}

	// Find first unoccupied location
	BallPosition newPosition;
	for (int i = 0; i < Grid::m_GridDimension; ++i)
	{
		for (int j = 0; j < Grid::m_GridDimension; ++j)
		{
			if (m_Grid.m_Cells[i][j].m_IsWalkable && !m_Grid.m_Cells[i][j].m_Occupied)
			{
				newPosition.x = i;
				newPosition.y = j;
				break;
			}
		}
	}

	return newPosition;
}

void CoreServer::FindPath()
{
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		BallPlayer& redPlayer = m_RedBallPlayers[i];
		BallPlayer& bluePlayer = m_BlueBallPlayers[i];
		int pathIndex = m_Grid.AStarPath(redPlayer, bluePlayer);
		GridPath path = m_Grid.m_Paths[pathIndex];
		m_RedBallPlayers[i].m_PathIndex = pathIndex;
		m_BlueBallPlayers[i].m_PathIndex = pathIndex;

		redPlayer.m_LastPositionIndex = 0;
		redPlayer.m_NextPositionIndex = 0;

		bluePlayer.m_LastPositionIndex = path.GetPathSize() - 1;
		bluePlayer.m_NextPositionIndex = path.GetPathSize() - 1;
	}
}

bool CoreServer::CheckIfRedTeamDead()
{
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		if (!m_RedBallPlayers[i].IsDead())
		{
			return false;
		}
	}

	return true;
}

bool CoreServer::CheckIfBlueTeamDead()
{
	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		if (!m_BlueBallPlayers[i].IsDead())
		{
			return false;
		}
	}

	return true;
}

void CoreServer::PerformHit(BallPlayer& player, BallPlayer& target)
{
	if (player.m_HitFrame == m_AttackFrame)
	{
		player.m_HitFrame = 0;
		--target.m_HitPoints;
	}
	else
	{
		++player.m_HitFrame;
	}
}

void CoreServer::Battle(BallPlayer& redPlayer, BallPlayer& bluePlayer)
{
	// One on one fight
	if (redPlayer.m_TargetEnemy == bluePlayer.m_PlayerId && bluePlayer.m_TargetEnemy == redPlayer.m_PlayerId)
	{
		if (redPlayer.m_IWillHitFirst)
		{
			PerformHit(redPlayer, bluePlayer);
			if (!bluePlayer.IsDead())
			{
				PerformHit(bluePlayer, redPlayer);
			}
		}
		else
		{
			PerformHit(bluePlayer, redPlayer);
			if (!redPlayer.IsDead())
			{
				PerformHit(redPlayer, bluePlayer);
			}
		}
	}
	// Red teaming up
	else if (redPlayer.m_TargetEnemy == bluePlayer.m_PlayerId)
	{
		PerformHit(redPlayer, bluePlayer);
	}
	// Blue teaming up
	else if (bluePlayer.m_TargetEnemy == redPlayer.m_PlayerId)
	{
		PerformHit(redPlayer, bluePlayer);
	}
	
}

void CoreServer::FollowPath(BallPlayer& player)
{
	if (player.m_MoveFrame < m_MoveTime)
	{
		++player.m_MoveFrame;
		player.m_BallMovement = BallMovement::Moving;
		return;
	}

	int pathIndex = m_Grid.UpdatePath(player);
	GridPath path = m_Grid.m_Paths[pathIndex];

	//UE_LOG(LogTemp, Warning, TEXT("Red path: %d --- Blue path: %d"), path.GetRedPlayerLocationIndex(), path.GetBluePlayerLocationIndex());

	if (!path.GetPathValid())
	{
		player.m_BallMovement = path.GetReached() ? BallMovement::TargetReached : BallMovement::Stationary;
		player.m_LastPositionIndex = player.m_NextPositionIndex;
	}
	else
	{
		if (player.GetBallTeam() == BallTeam::Red)
		{
			int index = path.GetRedPlayerLocationIndex();
			int prevIndex = index <= 0 ? 0 : index - 1;

			player.m_NextPositionIndex = index;
			player.m_LastPositionIndex = prevIndex;

		}
		else
		{
			int index = path.GetBluePlayerLocationIndex();
			int prevIndex = index == path.GetPathSize() - 1 ? path.GetPathSize() - 1 : index + 1;

			player.m_NextPositionIndex = index;
			player.m_LastPositionIndex = prevIndex;

		}
	}

	
	player.m_MoveFrame = 0;

}

void CoreServer::UpdateState()
{
	if (m_HasSimulationEnded)
	{
		return;
	}

	++m_FrameNumber;

	if (CheckIfBlueTeamDead() || CheckIfRedTeamDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("End Simulation"));
		if (CheckIfBlueTeamDead())
		{
			UE_LOG(LogTemp, Warning, TEXT("Red Won!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Blue Won!"));
		}

		m_HasSimulationEnded = true;
		return;
	}

	for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
	{
		BallPlayer& redPlayer = m_RedBallPlayers[i];
		BallPlayer& bluePlayer = m_BlueBallPlayers[redPlayer.m_TargetEnemy];

		if (redPlayer.HasTargetReached() && bluePlayer.HasTargetReached())
		{
			Battle(redPlayer, bluePlayer);
			UE_LOG(LogTemp, Warning, TEXT("Red health: %d --- Blue health: %d"), redPlayer.m_HitPoints, bluePlayer.m_HitPoints);
		}
		else
		{
			FollowPath(redPlayer);
			FollowPath(bluePlayer);
		}
		
	}
}

BallPosition CoreServer::GetBallNextPosition(const BallPlayer* player) const
{
	const GridPath& path = m_Grid.m_Paths[player->m_PathIndex];
	int playerPathIndex = player->m_NextPositionIndex;
	return path.GetPositionAt(playerPathIndex);
}

BallPosition CoreServer::GetBallLastPosition(const BallPlayer* player) const
{
	const GridPath& path = m_Grid.m_Paths[player->m_PathIndex];
	int playerPathIndex = player->m_LastPositionIndex;
	return path.GetPositionAt(playerPathIndex);
}

/////////////////////GRID/////////////////////
int Grid::ManhattanDistance(BallPosition start, BallPosition end)
{
	return ((abs(start.x - end.x) + abs(start.y - end.y)) * 10);
}

int Grid::AStarPath(const BallPlayer& red, const BallPlayer& blue)
{
	GridPath path;

	path.m_RedPlayerId = red.GetPlayerId();
	path.m_BluePlayerId = blue.GetPlayerId();

	BallPosition startPosition = red.GetCurrentPosition();
	BallPosition endPosition = blue.GetCurrentPosition();

	std::vector<BallPosition> openList;
	std::vector<BallPosition> closeList;

	openList.push_back(startPosition);

	auto FindLowestCostCell = [&]()
	{
		int result = 0;
		int fCost = (m_GridDimension + m_GridDimension + 2) * 10; // just a very big number that can't fit in the grid
		for (int i = 0; i < openList.size(); ++i)
		{
			BallPosition ballPos;
			ballPos.x = openList[i].x;
			ballPos.y = openList[i].y;

			if (fCost > m_Cells[ballPos.x][ballPos.y].m_fCost)
			{
				fCost = m_Cells[ballPos.x][ballPos.y].m_fCost;
				result = i;
			}
		}

		return result;
	};

	auto CalculateCostForOneNeighbor = [&](BallPosition& cellPosition, int neighborX, int neighborY)
	{
		int x = cellPosition.x;
		int y = cellPosition.y;

		GridCell& cell = m_Cells[x][y];

		int gCost = GridCell::m_StraightCost + cell.m_gCost;
		int hCost = ManhattanDistance(BallPosition(neighborX, neighborY), endPosition);
		int fCost = gCost + hCost;
		if (m_Cells[neighborX][neighborY].m_fCost != -1)
		{
			if (m_Cells[neighborX][neighborY].m_fCost > fCost)
			{
				m_Cells[neighborX][neighborY].m_ParentPosition.x = x;
				m_Cells[neighborX][neighborY].m_ParentPosition.y = y;
			}
			else if (m_Cells[neighborX][neighborY].m_fCost == fCost && m_Cells[neighborX][neighborY].m_hCost > hCost)
			{
				m_Cells[neighborX][neighborY].m_ParentPosition.x = x;
				m_Cells[neighborX][neighborY].m_ParentPosition.y = y;
			}
			else
			{
				openList.push_back(BallPosition(neighborX, neighborY));
				m_Cells[neighborX][neighborY].m_gCost = gCost;
				m_Cells[neighborX][neighborY].m_hCost = hCost;
				m_Cells[neighborX][neighborY].m_fCost = fCost;
			}
		}
		else
		{
			openList.push_back(BallPosition(neighborX, neighborY));
			m_Cells[neighborX][neighborY].m_gCost = gCost;
			m_Cells[neighborX][neighborY].m_hCost = hCost;
			m_Cells[neighborX][neighborY].m_fCost = fCost;
		}
	};

	auto CalculateCostForNeighbors = [&](BallPosition& cellPosition)
	{
		int x = cellPosition.x;
		int y = cellPosition.y;

		GridCell& cell = m_Cells[x][y];

		// North
		int neighborX = x;
		int neighborY = y + 1;
		if (neighborY < m_GridDimension)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// North - East
		neighborX = x + 1;
		neighborY = y + 1;
		if (neighborY < m_GridDimension && neighborX < m_GridDimension)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// East
		neighborX = x + 1;
		neighborY = y;
		if (neighborX < m_GridDimension)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// South - East
		neighborX = x + 1;
		neighborY = y - 1;
		if (neighborY >= 0 && neighborX < m_GridDimension)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// South
		neighborX = x;
		neighborY = y - 1;
		if (neighborY >= 0 )
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// South - West
		neighborX = x - 1;
		neighborY = y - 1;
		if (neighborY >= 0 && neighborX >= 0)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// West
		neighborX = x - 1;
		neighborY = y;
		if (neighborX >= 0)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

		// North - West
		neighborX = x - 1;
		neighborY = y + 1;
		if (neighborX < m_GridDimension && neighborX >= 0)
		{
			CalculateCostForOneNeighbor(cellPosition, neighborX, neighborY);
		}

	};

	while (!openList.empty())
	{
		// Find lowest fcost
		int lowestCostIndex = FindLowestCostCell();

		BallPosition closestCell = openList[lowestCostIndex];
		path.m_CommonPath.push_back(closestCell);
		if (closestCell == endPosition)
		{
			break;
		}
		openList.clear();

		CalculateCostForNeighbors(closestCell);
		
	}

	path.m_RedPlayerLocationIndex = 0;
	path.m_BluePlayerLocationIndex = path.m_CommonPath.size() - 1;

	m_Paths.push_back(path);

	return m_Paths.size() - 1;
}

int Grid::UpdatePath(BallPlayer& player)
{
	int index = 0;
	const bool isRed = player.GetBallTeam() == BallTeam::Red;
	for (int i = 0; i < m_Paths.size(); ++i)
	{
		if (isRed)
		{
			if (player.GetPlayerId() == m_Paths[i].m_RedPlayerId)
			{
				index = i;
				break;
			}
		}
		else
		{
			if (player.GetPlayerId() == m_Paths[i].m_BluePlayerId)
			{
				index = i;
				break;
			}
		}
	}

	GridPath& path = m_Paths[index];

	if (!path.m_PathValid)
	{
		return index;
	}

	if (abs(path.m_BluePlayerLocationIndex - path.m_RedPlayerLocationIndex) == 1)
	{
		path.m_Reached = true;
		path.m_PathValid = false;
		return index;
	}

	if (isRed)
	{
		++path.m_RedPlayerLocationIndex;
	}
	else
	{
		--path.m_BluePlayerLocationIndex;
	}

	return index;
}