#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <string>
#include <stack>
using namespace std;
// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
//class StudentWorld;
class StudentWorld;
class Actor : public GraphObject {
public:
    Actor(int imageID, int startX, int startY, StudentWorld* world, Direction startDirection, float size, unsigned int depth, string state = "");
    virtual ~Actor();
    virtual void doSomething() = 0;
    virtual void annoy(int points);
    virtual string getState();
    void updateState(string newState);
    bool alive();
    void setDead();
    void incTick();
    void resetTick();
    int getTick();
    StudentWorld* getWorld();
private:
    StudentWorld* m_world;
    bool m_alive;
    string m_state;
    int m_tick;
};

class Dirt : public Actor {
public:
    Dirt(int startX, int startY, StudentWorld* world, int imageID = IID_DIRT, Direction startDirection = right, float size = 0.25, unsigned int depth = 3);
    virtual ~Dirt();
    virtual void doSomething();
private:
};

class Boulder : public Actor {
public:
    Boulder(int startX, int startY, StudentWorld* world, int imageID = IID_BOULDER, Direction startDirection = down, float size = 1.0, unsigned int depth = 1);
    virtual ~Boulder();
    virtual void doSomething();
private:
};

class ActivatingObject : public Actor {
public:
    ActivatingObject(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth, string state = "");
    virtual ~ActivatingObject();
    virtual void doSomething() = 0;
    int getMaxTick();
private:
    int m_maxTick;
};

class Barrel : public ActivatingObject {
public:
    Barrel(int startX, int startY, StudentWorld* world, int imageID = IID_BARREL, Direction startDirection = right, float size = 1.0, unsigned int depth = 2);
    virtual ~Barrel();
    virtual void doSomething();
private:
};

class Gold : public ActivatingObject {
public:
    Gold(int startX, int startY, StudentWorld* world, string state, int imageID = IID_GOLD, Direction startDirection = right, float size = 1.0, unsigned int depth = 2);
    virtual ~Gold();
    virtual void doSomething();
private:
};

class Sonar : public ActivatingObject {
public:
    Sonar(int startX, int startY, StudentWorld* world, int imageID = IID_SONAR, Direction startDirection = right, float size = 1.0, unsigned int depth = 2);
    virtual ~Sonar();
    virtual void doSomething();
private:
};

class Water : public ActivatingObject {
public:
    Water(int startX, int startY, StudentWorld* world, int imageID = IID_WATER_POOL, Direction startDirection = right, float size = 1.0, unsigned int depth = 2);
    virtual ~Water();
    virtual void doSomething();
private:
};

class Squirt : public Actor {
public:
    Squirt(int startX, int startY, StudentWorld* world, Direction startDirection, int imageID = IID_WATER_SPURT, float size = 1.0, unsigned int depth = 1);
    virtual ~Squirt();
    virtual void doSomething();
private:
    int m_distance;
};

class FrackMan : public Actor {
public:
    FrackMan(int startX, int startY, StudentWorld* world, int imageID = IID_PLAYER, Direction startDirection = right, float size = 1.0, unsigned int depth = 0);
    virtual ~FrackMan();
    virtual void doSomething();
    virtual void annoy(int points);
    int getHp();
    int getWater();
    int getSonar();
    int getGold();
    void changeHp(int change);
    void changeWater(int change);
    void changeSonar(int change);
    void changeGold(int change);
private:
    int m_hp;
    int m_water;
    int m_sonar;
    int m_gold;
};

class Protester : public Actor {
public:
    Protester(int startX, int startY, StudentWorld* world, int hp, int imageID, Direction startDirection, float size, unsigned int depth);
    virtual ~Protester();
    virtual void doSomething() = 0;
    virtual void annoy(int points) = 0;
    int getHp();
    void changeHp(int change);
    bool facingFrackMan(int x, int y, Direction direction);
    bool inSight(int x, int y);
    bool canMakeTurn(int x, int y, Direction current);
    Direction makePerpendicularTurn(int x, int y, Direction current);
    Direction pickNewDirection();
    Direction chooseNotBlockedDirection(Direction start, int x, int y);
    void checkDirtBoulder(int& x, int& y, int& dx, int& dy, Direction start);
private:
    int m_hp;
};

class Regular : public Protester {
public:
    Regular(int startX, int startY, StudentWorld* world, int imageID = IID_PROTESTER, Direction startDirection = left, float size = 1.0, unsigned int depth = 0);
    virtual ~Regular();
    virtual void doSomething();
    virtual void annoy(int points);
private:
    int m_ticksToWaitBetweenMoves;
    bool m_canShout;
    int m_ticksToShout;
    bool m_canTurn;
    int m_ticksToTurn;
    bool m_stun;
    int m_ticksToUnstun;
    int m_restingTicks;
    int m_numSquaresToMoveInCurrentDirection;
};

class Hard : public Protester {
public:
    Hard(int startX, int startY, StudentWorld* world, int imageID = IID_HARD_CORE_PROTESTER, Direction startDirection = left, float size = 1.0, unsigned int depth = 0);
    virtual ~Hard();
    virtual void doSomething();
    virtual void annoy(int points);
private:
    int m_ticksToWaitBetweenMoves;
    bool m_canShout;
    int m_ticksToShout;
    bool m_canTurn;
    int m_ticksToTurn;
    bool m_stun;
    int m_ticksToUnstun;
    int m_restingTicks;
    int m_numSquaresToMoveInCurrentDirection;
};
#endif // ACTOR_H_
