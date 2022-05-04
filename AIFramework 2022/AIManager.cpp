#include "AIManager.h"
#include "Vehicle.h"
#include "DrawableGameObject.h"
#include "PickupItem.h"
#include "Waypoint.h"
#include "main.h"
#include "constants.h"
#include<windows.h>
#include <string>
#include <queue>
#include <map>
struct Node
{
    Waypoint* data;
    vecWaypoints neighbors;
};




AIManager::AIManager()
{
	m_pCar = nullptr;
    m_rCar = nullptr;
}

AIManager::~AIManager()
{
	release();
}

void AIManager::release()
{
	clearDrawList();

	for (PickupItem* pu : m_pickups)
	{
		delete pu;
	}
	m_pickups.clear();

	delete m_pCar;
	m_pCar = nullptr;

    delete m_rCar;
    m_rCar = nullptr;
}

HRESULT AIManager::initialise(ID3D11Device* pd3dDevice)
{
     // set state to change stering and stragey AI
    SetState = STERRING;

    setStrState = SeekPassanger;
    fuel = 200;
    m_arrived = false;
    m_arrive = false;
    flee = false;
    obset = false;
    

    // create the vehicle 
    float xPos = -500; // an abtrirary start point
    float yPos = 300;

    m_pCar = new Vehicle();
    m_rCar = new Vehicle();

    HRESULT hr = m_pCar->initMesh(pd3dDevice, carColour::blueCar);
    HRESULT hr2 = m_rCar->initMesh(pd3dDevice, carColour::redCar);

    m_pCar->setVehiclePosition(Vector2D(xPos, yPos));
    m_rCar->setVehiclePosition(Vector2D(0, 0));

    if (FAILED(hr))
        return hr;

    // setup the waypoints
    m_waypointManager.createWaypoints(pd3dDevice);
    m_pCar->setWaypointManager(&m_waypointManager);
    m_rCar->setWaypointManager(&m_waypointManager);

    // create a passenger pickup item
    PickupItem* pPickupPassenger = new PickupItem();
    hr = pPickupPassenger->initMesh(pd3dDevice, pickuptype::Passenger);
    m_pickups.push_back(pPickupPassenger);

    // NOTE!! for fuel and speedboost - you will need to create these here yourself
    PickupItem* pPickupFuel = new PickupItem();
    hr = pPickupFuel->initMesh(pd3dDevice, pickuptype::Fuel);
    m_pickups.push_back(pPickupFuel);

    PickupItem* pPickupSpeedBoost = new PickupItem();
    hr = pPickupSpeedBoost->initMesh(pd3dDevice, pickuptype::SpeedBoost);
    m_pickups.push_back(pPickupSpeedBoost);


    // (needs to be done after waypoint setup)
    setRandomPickupPosition(pPickupPassenger);
    setRandomPickupPosition(pPickupFuel);
    setRandomPickupPosition(pPickupSpeedBoost);
    //setRandomWaypoint(m_bCar);

    location = m_pCar->getPosition();
    StartLoc = m_waypointManager.getNearestWaypoint(location);
    endLoc = m_waypointManager.getNearestWaypoint(Vector2D(0,0));

    
    m_pCar->setCurrentSpeed(0.7);
    return hr;
}


void AIManager::update(const float fDeltaTime)
{
    for (unsigned int i = 0; i < m_waypointManager.getWaypointCount(); i++) {
        m_waypointManager.getWaypoint(i)->update(fDeltaTime);
        //AddItemToDrawList(m_waypointManager.getWaypoint(i)); // if you uncomment this, it will display the waypoints
    }

    for (int i = 0; i < m_waypointManager.getQuadpointCount(); i++)
    {
        Waypoint* qp = m_waypointManager.getQuadpoint(i);
        qp->update(fDeltaTime);
        //AddItemToDrawList(qp); // if you uncomment this, it will display the quad waypoints
    }

    // update and display the pickups
    for (unsigned int i = 0; i < m_pickups.size(); i++) {
        m_pickups[i]->update(fDeltaTime);
        AddItemToDrawList(m_pickups[i]);
    }

	// draw the waypoints nearest to the car
	/*
    Waypoint* wp = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());
	if (wp != nullptr)
	{
		vecWaypoints vwps = m_waypointManager.getNeighbouringWaypoints(wp);
		for (Waypoint* wp : vwps)
		{
			AddItemToDrawList(wp);
		}
	}
    */

   

    // update and draw the car (and check for pickup collisions)
	if (m_pCar != nullptr && m_rCar != nullptr)
	{
        wan++;

		m_pCar->update(fDeltaTime);
        m_rCar->update(fDeltaTime);
		checkForCollisions();
		AddItemToDrawList(m_pCar);
        AddItemToDrawList(m_rCar);

        if (aiRedState == MOVING)
        {
            if (wan >= 50)
            {
                setRandomWaypoint(m_rCar);
                wan = 0;
                
                    
            }
        }
        
        if (blueState == FOLLOW)
        {
            followRedCar();
        }
	}

    //Stragtegy mode
    if (SetState == STRATEGY)
    {
        strategy();
        fuel--;
        
       
        
    }
    
    if(flee)
    Flee();
    
    if (m_arrive)
        Arrive();
   
    if(obset)
        obstacleAvoidance();

}

void AIManager::mouseUp(int x, int y)
{
	// get a waypoint near the mouse click, then set the car to move to the this waypoint
	Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
	if (wp == nullptr)
		return;

    // steering mode
    m_pCar->setPositionTo(wp->getPosition());
}

void AIManager::keyUp(WPARAM param)
{
    const WPARAM key_a = 65;
    switch (param)
    {
        case key_a:
        {
            OutputDebugStringA("a Up \n");
            break;
        }
    }
}

void AIManager::keyDown(WPARAM param)
{
	// hint 65-90 are a-z
	const WPARAM key_a = 65;
	const WPARAM key_s = 83;
    const WPARAM key_t = 84;
    const WPARAM key_w = 87;
    const WPARAM key_p = 80;
    const WPARAM key_o = 79;
    const WPARAM key_f = 70;
   
    if (SetState == STERRING)
    {
        switch (param)
        {
        case VK_NUMPAD0:
        {
            OutputDebugStringA("0 pressed \n");
            break;
        }
        case VK_NUMPAD1:
        {
            OutputDebugStringA("1 pressed \n");
            break;
        }
        case VK_NUMPAD2:
        {
            OutputDebugStringA("2 pressed \n");
            break;
        }
        case key_a:
        {
            int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
            int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

            ArriveWP = m_waypointManager.getNearestWaypoint(Vector2D(x, y));

            m_pCar->setPositionTo(ArriveWP->getPosition());

            if(!m_arrive)
            m_arrive = true;
            
            

            Arrive();
           

            break;
        }
        case key_s:
        {
            //blue car to go to a random waypoint
            setRandomWaypoint(m_pCar);
            break;
        }
        case key_t:
        {
            //move blue car to red cars postion
            m_pCar->setPositionTo(m_rCar->getPosition());

            break;
        }
        case key_w:
        {
            //toggle red car wander 
            if (aiRedState == IDLE)
                aiRedState = MOVING;
            else if (aiRedState == MOVING)
            {
                aiRedState = IDLE;
            }


            break;
        }
        case key_p:
        {
            //follow red car
            if (blueState == Idle)
            {
                blueState = FOLLOW;
            }
            else if (blueState == FOLLOW)
            {
                blueState = Idle;
            }
            break;
        }
        case key_f:
        {
            if (!flee)
            {
                flee = true;
                OutputDebugStringA("flee on \n");
            }
            else
            {
                flee = false;
                OutputDebugStringA("flee off \n");
            }
            break;
        }
        case key_o:
        {
            int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
            int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

            obwp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));

            if (!obset)
                obset = true;
            

            break;
        }


        case VK_SPACE:
            PathFinding();

            break;
// etc
        default:
            break;
        }
    }
}

void AIManager::setRandomPickupPosition(PickupItem* pickup)
{
    if (pickup == nullptr)
        return;

    int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
    int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

    Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));
    if (wp) {
        pickup->setPosition(wp->getPosition());
    }
}

void  AIManager::setRandomWaypoint(Vehicle* vehi)
{


    int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
    int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

    Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(x, y));

    vehi->setPositionTo(wp->getPosition());

    if (vehi->getPosition() == wp->getPosition())
    {
        vehi->setPositionTo(wp->getPosition());
    }


}


void AIManager::followRedCar()
{
   
    Waypoint* wp = m_waypointManager.getNearestWaypoint(m_rCar->getPosition());

    
        m_pCar->setPositionTo(wp->getPosition());


}

void AIManager::Flee()
{
    Waypoint* blueCar = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());

    Waypoint* redCar = m_waypointManager.getNearestWaypoint(m_rCar->getPosition());
    

    int x = (rand() % SCREEN_WIDTH) - (SCREEN_WIDTH / 2);
    int y = (rand() % SCREEN_HEIGHT) - (SCREEN_HEIGHT / 2);

    Waypoint* wp = m_waypointManager.getNearestWaypoint(Vector2D(-491, 363));




    if (blueCar->distanceToWaypoint(redCar) < 200)
    {
        m_pCar->setPositionTo(wp->getPosition());

    }
    else if (blueCar->distanceToWaypoint(redCar) > 220)
    {
        m_pCar->setPositionTo(m_pCar->getPosition());
    }


}



void AIManager::Arrive()
{

    Waypoint* blueCar = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());




    if (blueCar->distanceToWaypoint(ArriveWP) < 500 && blueCar->distanceToWaypoint(ArriveWP) > 400)
    {
        m_pCar->setCurrentSpeed(0.7);
        

    }
    else if (blueCar->distanceToWaypoint(ArriveWP) < 400 && blueCar->distanceToWaypoint(ArriveWP) > 300)
    {
        m_pCar->setCurrentSpeed(0.6);
    
    }
    else if (blueCar->distanceToWaypoint(ArriveWP) < 300 && blueCar->distanceToWaypoint(ArriveWP) >= 150)
    {
       
        m_pCar->setCurrentSpeed(0.5);
    }
    else if (blueCar->distanceToWaypoint(ArriveWP) < 150 && blueCar->distanceToWaypoint(ArriveWP) >= 50)
    {
        
        m_pCar->setCurrentSpeed(0.3);
    }
    else if (blueCar->distanceToWaypoint(ArriveWP) < 50 && blueCar->distanceToWaypoint(ArriveWP) >= 0)
    {
       
        m_pCar->setCurrentSpeed(0.1);
    }
   
    
    
    if (m_pCar->getPosition() == ArriveWP->getPosition())
    {
        m_arrive = false;
        m_pCar->setCurrentSpeed(0.8);
    }
}


void AIManager::obstacleAvoidance()
{
    Waypoint* blueCar = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());
    Waypoint* redCar = m_waypointManager.getNearestWaypoint(m_rCar->getPosition());
   

    if (blueCar->distanceToWaypoint(redCar) <= 100)
    {
        m_pCar->setPositionTo(Vector2D(200, 200));

    }
    else if (blueCar->distanceToWaypoint(redCar) >= 150)
    {
        m_pCar->setPositionTo(obwp->getPosition());
    }

    if (m_pCar->getPosition() == obwp->getPosition())
    {
        obset = false;
        
    }
}


void AIManager::PathFinding()
{
    queue<Node*> frontier;
    map<Node*, Node*> came_from;

    Node* startNode = new Node();
    startNode->data = StartLoc;
    Node* endNode = new Node();
    endNode->data = endLoc;


    

    Node* currentNeighbors = new Node();
    currentNeighbors->neighbors = m_waypointManager.getNeighbouringWaypoints(StartLoc);
    

    frontier.push(startNode);
    
    while (!frontier.empty())
    {
        
        Node* current = frontier.front();
        frontier.pop();

        if (current == endNode) break;

        for (Node* neighbours ; currentNeighbors;)
        {
            
            if (!came_from[neighbours])
            {
                frontier.push(neighbours);
                came_from.insert({ neighbours, current});
            }
            
           
        }
    }
    
}




void AIManager::strategy()
{
    Waypoint* wpfuel = m_waypointManager.getNearestWaypoint(m_pickups[1]->getPosition());
    Waypoint* wpPassenger = m_waypointManager.getNearestWaypoint(m_pickups[0]->getPosition());
    Waypoint* wpBoost = m_waypointManager.getNearestWaypoint(m_pickups[2]->getPosition());
    Waypoint* blueCar = m_waypointManager.getNearestWaypoint(m_pCar->getPosition());

    if (wpfuel->distanceToWaypoint(blueCar) < wpPassenger->distanceToWaypoint(blueCar) && wpfuel->distanceToWaypoint(blueCar) < wpBoost->distanceToWaypoint(blueCar))
    {
        setStrState = SeekFual;
    }
     else if (wpBoost->distanceToWaypoint(blueCar) < wpPassenger->distanceToWaypoint(blueCar) && wpBoost->distanceToWaypoint(blueCar) < wpfuel->distanceToWaypoint(blueCar))
    {
        setStrState = SeekBoost;
    }
     else if (wpPassenger->distanceToWaypoint(blueCar) < wpBoost->distanceToWaypoint(blueCar) && wpPassenger->distanceToWaypoint(blueCar) < wpfuel->distanceToWaypoint(blueCar))
    {
        setStrState = SeekPassanger;
    }
    if (fuel <= 50)
    {
        OutputDebugStringA("Fuel is LOW \n");
        setStrState = SeekFual;
        
    }
    if (fuel <= 0)
    {
        OutputDebugStringA("Fuel is Empty \n");
        m_pCar->setCurrentSpeed(0.2);
    }


    switch (setStrState)
    {
    case SeekPassanger:
            m_pCar->setPositionTo(m_pickups[0]->getPosition());
        
        break;

    case SeekBoost:
        
        
            m_pCar->setPositionTo(m_pickups[2]->getPosition());
       
        
        break;
    case SeekFual:
        
            m_pCar->setPositionTo(m_pickups[1]->getPosition());

        break;

    default:
        break;
    }
}

/*
// IMPORTANT
// hello. This is hopefully the only time you are exposed to directx code 
// you shouldn't need to edit this, but marked in the code below is a section where you may need to add code to handle pickup collisions (speed boost / fuel)
// the code below does the following:
// gets the *first* pickup item "m_pickups[0]"
// does a collision test with it and the car
// creates a new random pickup position for that pickup

// the relevant #includes are already in place, but if you create your own collision class (or use this code anywhere else) 
// make sure you have the following:
#include <d3d11_1.h> // this has the appropriate directx structures / objects
#include <DirectXCollision.h> // this is the dx collision class helper
using namespace DirectX; // this means you don't need to put DirectX:: in front of objects like XMVECTOR and so on. 
*/

bool AIManager::checkForCollisions()
{
    if (m_pickups.size() == 0)
        return false;

    XMVECTOR dummy;

    // get the position and scale of the car and store in dx friendly xmvectors
    XMVECTOR carPos;
    XMVECTOR carScale;
    XMMatrixDecompose(
        &carScale,
        &dummy,
        &carPos,
        XMLoadFloat4x4(m_pCar->getTransform())
    );

    // create a bounding sphere for the car
    XMFLOAT3 scale;
    XMStoreFloat3(&scale, carScale);
    BoundingSphere boundingSphereCar;
    XMStoreFloat3(&boundingSphereCar.Center, carPos);
    boundingSphereCar.Radius = scale.x;

    // do the same for a pickup item
    // a pickup - !! NOTE it is only referring the first one in the list !!
    // to get the passenger, fuel or speedboost specifically you will need to iterate the pickups and test their type (getType()) - see the pickup class
    XMVECTOR puPos;
    XMVECTOR puScale;
    
     XMMatrixDecompose(
            &puScale,
            &dummy,
            &puPos,
            XMLoadFloat4x4(m_pickups[0]->getTransform())
        );
    // bounding sphere for pickup item
    XMStoreFloat3(&scale, puScale);
    BoundingSphere boundingSpherePa;
    XMStoreFloat3(&boundingSpherePa.Center, puPos);
    boundingSpherePa.Radius = scale.x;

    //fuel pickup
    XMVECTOR puPos1;
    XMVECTOR puScale1;
    XMMatrixDecompose(
        &puScale1,
        &dummy,
        &puPos1,
        XMLoadFloat4x4(m_pickups[1]->getTransform())
    );
    XMStoreFloat3(&scale, puScale1);
    BoundingSphere boundingSpherePa1;
    XMStoreFloat3(&boundingSpherePa1.Center, puPos1);
    boundingSpherePa1.Radius = scale.x;

    //speed PickUp
    XMVECTOR puPos2;
    XMVECTOR puScale2;
    XMMatrixDecompose(
        &puScale2,
        &dummy,
        &puPos2,
        XMLoadFloat4x4(m_pickups[2]->getTransform())
    );
    XMStoreFloat3(&scale, puScale2);
    BoundingSphere boundingSpherePa2;
    XMStoreFloat3(&boundingSpherePa2.Center, puPos2);
    boundingSpherePa2.Radius = scale.x;
	// THIS IS generally where you enter code to test each type of pickup
    // does the car bounding sphere collide with the pickup bounding sphere?
    //passenger coll
    if (boundingSphereCar.Intersects(boundingSpherePa))
    {
        OutputDebugStringA("Passenger collected\n");
        m_pickups[0]->hasCollided();
        setRandomPickupPosition(m_pickups[0]);
       
        m_pCar->setCurrentSpeed(0.7);
        // you will need to test the type of the pickup to decide on the behaviour
        // m_pCar->dosomething(); ...
        return true;
    }
    //fuel coll
    else if (boundingSphereCar.Intersects(boundingSpherePa1))
    {
        OutputDebugStringA("Fuel Colleted \n");
        m_pickups[1]->hasCollided();
        setRandomPickupPosition(m_pickups[1]);
        fuel = 200;
       
        m_pCar->setCurrentSpeed(0.7);

        return true;
    }
    //speed boost coll
    else if (boundingSphereCar.Intersects(boundingSpherePa2))
    {
        OutputDebugStringA("Speed boost collected\n");
        m_pickups[2]->hasCollided();
        setRandomPickupPosition(m_pickups[2]);
        
        m_pCar->setCurrentSpeed(1);
        return true;
    }
    return false;
}





