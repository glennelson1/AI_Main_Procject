#pragma once

#include "WaypointManager.h"

using namespace std;

class Vehicle;
class PickupItem;
typedef vector<PickupItem*> vecPickups;

class AIManager
{
public:
	AIManager();
	virtual  ~AIManager();
	void	release();
	HRESULT initialise(ID3D11Device* pd3dDevice);
	void	update(const float fDeltaTime);
	void	mouseUp(int x, int y);
	void	keyDown(WPARAM param);
	void	keyUp(WPARAM param);

protected:
	bool	checkForCollisions();
	void	setRandomPickupPosition(PickupItem* pickup);
	void    setRandomWaypoint(Vehicle* vehi);

	void    followRedCar();
	void    Arrive();

	int wan ;
	
	Vector2D location;
	Vector2D nextloc;
	Waypoint* StartLoc;
	Waypoint* endLoc;

	enum state {WANDER, STERRING, PATHFINDING, STRATEGY};

	Waypoint*  PathFinding();

	enum BlueState { Idle, FOLLOW};
	BlueState blueState;

	enum Redstate { IDLE, MOVING};
	Redstate aiRedState;

private:
	vecPickups              m_pickups;
	Vehicle*				m_pCar = nullptr;
	Vehicle*                m_rCar = nullptr;
	WaypointManager			m_waypointManager;

};

