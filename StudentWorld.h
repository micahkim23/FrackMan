#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include "GameController.h"
#include <string>
#include <vector>
#include <stack>
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Coord
{
public:
    Coord(int rr, int cc, int dd) : m_r(rr), m_c(cc), m_d(dd) {}
    int r() const { return m_r; }
    int c() const { return m_c; }
    int d() const { return m_d; }
private:
    int m_r;
    int m_c;
    int m_d;
};

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    bool distToFracmanGreaterThanN(int x, int y, int n);
    bool distGreaterThanNSquares(int x, int y, int n, bool objSpecific, int imageID, bool otherObj);
    bool dirtExistsInBox(int x, int y, int dx, int dy);
    bool boulderExistsInBox(int x, int y, int dx, int dy);
    bool poolExistsInBox(int x, int y, int dx, int dy);
    bool objectAnnoyProtester(int bX, int bY, int points);
    bool objectAnnoyHardProtester(int bX, int bY, int points);
    bool deleteDirt(int startX, int startY);
    Actor* bribeProtester(int x, int y);
    FrackMan* getFrackMan();
    Actor::Direction getOptimalPath(int x, int y);
    Actor::Direction getPathToFrackMan(int x, int y);
    void findOptimalPath(Actor::Direction maze[61][61], int sr, int sc, int depth);
    void callFindOptimalPath();
    std::vector<Actor*>* getObjects();
    void setVisibleWithinDistanceN(int n);
    void decNumProtesters();
    int min(int a, int b);
    int max(int a, int b);
    int randInt(int min, int max);
    int abs(int a);
private:
    // data
    Dirt* m_dirt[64][60];
    Actor::Direction m_optimalPath[61][61];
    Actor::Direction m_pathToFrackman[61][61];
    FrackMan* m_man;
    std::vector<Actor*> m_object;
    int m_tick;
    int m_numProtesters;
    // functions
    void resetPosition(int& x, int& y);
    int getNumberOfBarrelsRemainingToBePickedUp();
    bool theplayerCompletedTheCurrentLevel();
    void setDisplayText();
    std::string scorePadding(string score);
    std::string statPadding(int statistic, int lessThan);
    void addSonarWater();
    void addProtester();
};

#endif // STUDENTWORLD_H_
