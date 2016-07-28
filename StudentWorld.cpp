#include "StudentWorld.h"
#include <string>
#include <vector>
#include <random>
#include <queue>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(string assetDir)
: GameWorld(assetDir), m_tick(0), m_numProtesters(0)
{
}

StudentWorld::~StudentWorld() {
    // delete FrackMan
    delete m_man;
    // delete Dirt
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            delete m_dirt[i][j];
        }
    }
    // delete Actors
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        delete (*p);
    }
    m_object.clear();
}

int StudentWorld::init() {
    // initialize dirt objects
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            if (j < 4 || i < 30 || i > 33) 
                m_dirt[i][j] = new Dirt(i, j, this);
            else
                m_dirt[i][j] = nullptr;
        }
    }
    // initialize FrackMan
    m_man = new FrackMan(30, 60, this);
    // initialize Boulder
    int B = min(getLevel()/2 + 2, 6);
    for (int i = 0; i < B; i++) {
        int x = randInt(0,60);
        int y = randInt(20,56);
        resetPosition(x, y);
        m_object.push_back(new Boulder(x, y, this));
        deleteDirt(x, y);
    }
    // initialize Barrel
    int L = min(2 + getLevel(), 20);
    for (int i = 0; i < L; i++) {
        int x = randInt(0,60);
        int y = randInt(20,56);
        resetPosition(x, y);
        m_object.push_back(new Barrel(x, y, this));
    }
    // initialize Gold
    int G = max(5-getLevel()/2, 2);
    for (int i = 0; i < G; i++) {
        int x = randInt(0,60);
        int y = randInt(20,56);
        resetPosition(x, y);
        m_object.push_back(new Gold(x, y, this, "permanent"));
    }
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
    setDisplayText();
    addSonarWater();
    addProtester();
    findOptimalPath(m_pathToFrackman, m_man->getX(), m_man->getY(),16+getLevel()*2);
    m_man->doSomething();
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        (*p)->doSomething();
    }
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if (!(*p)->alive()) {
            delete *p;
            vector<Actor*>::iterator q = m_object.erase(p);
            p = q;
            p--;
        }
    }
    m_tick++;
    if (!m_man->alive()) {
        m_tick = 0;
        m_numProtesters = 0;
        return GWSTATUS_PLAYER_DIED;
    }
    if (theplayerCompletedTheCurrentLevel()) {
        GameController::getInstance().playSound(SOUND_FINISHED_LEVEL);
        m_tick = 0;
        m_numProtesters = 0;
        return GWSTATUS_FINISHED_LEVEL;
    }
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
    delete m_man;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 60; j++) {
            delete m_dirt[i][j];
        }
    }
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        delete (*p);
    }
    m_object.clear();
}

//public helper functions
bool StudentWorld::distToFracmanGreaterThanN(int x, int y, int n) {
    double dx = m_man->getX() - x;
    double dy = m_man->getY() - y;
    if (dx*dx + dy*dy <= n*n)
        return false;
    return true;
}

bool StudentWorld::distGreaterThanNSquares(int x, int y, int n, bool objSpecific, int imageID, bool otherObj) {
    // checks that Euclidean distance is less than n
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        double dx = (*p)->getX() - x;
        double dy = (*p)->getY() - y;
        if (dx*dx + dy*dy <= n*n) {
            // if user wants to only compare distances with one type of object
            if (objSpecific) {
                if ((*p)->getID() == imageID) {
                    // if user wants to only compare distances with other objects of same type
                    // pass in actual x and y of object
                    if (otherObj) {
                        if (x != (*p)->getX() || y != (*p)->getY()) {
                            return false;
                        }
                    }
                    else {
                        return false;
                    }
                }
            }
            else {
                return false;
            }
        }
    }
    return true;
}

bool StudentWorld::dirtExistsInBox(int x, int y, int dx, int dy) {
    for (int i = x; i < x + dx && i < 64; i++) {
        for (int j = y; j < y + dy && j < 60; j++) {
            if (m_dirt[i][j] != nullptr) {
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::boulderExistsInBox(int x, int y, int dx, int dy) {
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if ((*p)->getID() == IID_BOULDER) {
            int bX = (*p)->getX();
            int bY = (*p)->getY();
            if (bX > x - 4 && bX < x + dx && bY > y - 4 && bY < y + dy)
                return true;
        }
    }
    return false;
}

bool StudentWorld::poolExistsInBox(int x, int y, int dx, int dy) {
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if ((*p)->getID() == IID_WATER_POOL) {
            int bX = (*p)->getX();
            int bY = (*p)->getY();
            if (bX > x - 4 && bX < x + dx && bY > y - 4 && bY < y + dy)
                return true;
        }
    }
    return false;
}

bool StudentWorld::objectAnnoyProtester(int bX, int bY, int points) {
    bool hit = false;
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if ((*p)->getID() == IID_PROTESTER && (*p)->getState() != "leave") {
            int dX = (*p)->getX() - bX;
            int dY = (*p)->getY() - bY;
            if (dX*dX + dY*dY <= 9) {
                (*p)->annoy(points);
                hit = true;
            }
        }
    }
    return hit;
}

bool StudentWorld::objectAnnoyHardProtester(int bX, int bY, int points) {
    bool hit = false;
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if ((*p)->getID() == IID_HARD_CORE_PROTESTER && (*p)->getState() != "leave") {
            int dX = (*p)->getX() - bX;
            int dY = (*p)->getY() - bY;
            if (dX*dX + dY*dY <= 9) {
                (*p)->annoy(points);
                hit = true;
            }
        }
    }
    return hit;
}

bool StudentWorld::deleteDirt(int startX, int startY) {
    bool destroy_dirt = false;
    for (int i = startX; i <= startX + 3; i++) {
        for (int j = startY; j <= startY + 3 && j <= 59; j++) {
            if (m_dirt[i][j] != nullptr)
                destroy_dirt = true;
            delete m_dirt[i][j];
            m_dirt[i][j] = nullptr;
        }
    }
    return destroy_dirt;
}

Actor* StudentWorld::bribeProtester(int x, int y) {
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if ((*p)->getID() == IID_HARD_CORE_PROTESTER || (*p)->getID() == IID_PROTESTER) {
            int dx = (*p)->getX() - x;
            int dy = (*p)->getY() - y;
            if (dx*dx + dy*dy <= 9)
                return *p;
        }
    }
    return nullptr;
}

FrackMan* StudentWorld::getFrackMan() {
    return m_man;
}

Actor::Direction StudentWorld::getOptimalPath(int x, int y) {
    return m_optimalPath[x][y];
}

vector<Actor*>* StudentWorld::getObjects() {
    return &m_object;
}

void StudentWorld::setVisibleWithinDistanceN(int n) {
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        int x = (*p)->getX();
        int y = (*p)->getY();
        if (!distToFracmanGreaterThanN(x, y, 12))
            (*p)->setVisible(true);
    }
}

void StudentWorld::findOptimalPath(Actor::Direction maze[61][61], int sr, int sc, int depth) {
    for (int i = 0; i < 61; i++) {
        for (int j = 0; j < 61; j++) {
            maze[i][j] = Actor::none;
        }
    }
    queue<Coord> coordQueue;
    coordQueue.push(Coord(sr,sc,0));
    m_optimalPath[sr][sc] = Actor::right;
    while (!coordQueue.empty()) {
        Coord curr = coordQueue.front();
        coordQueue.pop();
        int r = curr.r();
        int c = curr.c();
        int d = curr.d();
        // move North
        if (c < 60 && maze[r][c+1] == Actor::none && !dirtExistsInBox(r, c+4, 4, 1) && !boulderExistsInBox(r, c+4, 4, 1) && d < depth) {
            coordQueue.push(Coord(r, c+1,d+1));
            maze[r][c+1] = Actor::down;
        }
        // move East
        if (r < 60 && maze[r+1][c] == Actor::none && !dirtExistsInBox(r+4, c, 1, 4) && !boulderExistsInBox(r+4, c, 1, 4) && d < depth) {
            coordQueue.push(Coord(r+1,c,d+1));
            maze[r+1][c] = Actor::left;
        }
        // move South
        if (c > 0 && maze[r][c-1] == Actor::none && !dirtExistsInBox(r, c-1, 4, 1) && !boulderExistsInBox(r, c-1, 4, 1) && d < depth) {
            coordQueue.push(Coord(r,c-1,d+1));
            maze[r][c-1] = Actor::up;
        }
        // move West
        if (r > 0 && maze[r-1][c] == Actor::none && !dirtExistsInBox(r-1, c, 1, 4) && !boulderExistsInBox(r-1, c, 1, 4) && d < depth) {
            coordQueue.push(Coord(r-1,c,d+1));
            maze[r-1][c] = Actor::right;
        }
    }
}

void StudentWorld::callFindOptimalPath() {
    findOptimalPath(m_optimalPath, 60, 60, 10000);
}

void StudentWorld::decNumProtesters() {
    m_numProtesters--;
}

int StudentWorld::min(int a, int b) {
    if (a < b)
        return a;
    return b;
}

int StudentWorld::max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

int StudentWorld::randInt(int min, int max)
{
    if (max < min)
        swap(max, min);
    static random_device rd;
    static mt19937 generator(rd());
    uniform_int_distribution<> distro(min, max);
    return distro(generator);
}

int StudentWorld::abs(int a) {
    if (a < 0)
        return a * -1;
    return a;
}

Actor::Direction StudentWorld::getPathToFrackMan(int x, int y) {
    return m_pathToFrackman[x][y];
}

// private helper functions

void StudentWorld::resetPosition(int& x, int& y) {
    while (!distGreaterThanNSquares(x, y, 6, false, 0, false) || (x > 26 && x < 34)) {
        x = randInt(0,60);
        y = randInt(20,56);
    }
}

int StudentWorld::getNumberOfBarrelsRemainingToBePickedUp() {
    int numBarrels = 0;
    for (vector<Actor*>::iterator p = m_object.begin(); p != m_object.end(); p++) {
        if ((*p)->getID() == IID_BARREL)
            numBarrels++;
    }
    return numBarrels;
}

bool StudentWorld::theplayerCompletedTheCurrentLevel() {
    return getNumberOfBarrelsRemainingToBePickedUp() == 0;
}

void StudentWorld::setDisplayText() {
    int score = getScore();
    int level = getLevel();
    int lives = getLives();
    int health = m_man->getHp();
    int squirts = m_man->getWater();
    int gold = m_man->getGold();
    int sonar = m_man->getSonar();
    int barrelsLeft = getNumberOfBarrelsRemainingToBePickedUp();
    string s = "Scr: " + scorePadding(to_string(score)) + "  Lvl: " + statPadding(level, 10) + "  Lives: " + to_string(lives) + "  Hlth: " + statPadding(10*health, 100) + "%  Wtr: " + statPadding(squirts, 10) + "  Gld: " + statPadding(gold, 10) + "  Sonar: " + statPadding(sonar,10) + "  Oil Left: " + statPadding(barrelsLeft, 10);
    setGameStatText(s);
    
}

string StudentWorld::scorePadding(string score) {
    while (score.size() < 6)
        score = "0" + score;
    return score;
}

string StudentWorld::statPadding(int statistic, int lessThan) {
    if (statistic < lessThan)
        return " " + to_string(statistic);
    return to_string(statistic);
}

void StudentWorld::addSonarWater() {
    int G = getLevel()*25+300;
    int num = randInt(1, G);
    if (num == 1) {
        int num2 = randInt(1,5);
        if (num2 == 1) {
            m_object.push_back(new Sonar(0, 60, this));
        }
        else {
            int x = randInt(0,60);
            int y = randInt(0,56);
            while (dirtExistsInBox(x, y, 4, 4) || poolExistsInBox(x, y, 4, 4)) {
                x = randInt(0,60);
                y = randInt(0,56);
            }
            m_object.push_back(new Water(x, y, this));
        }
    }
}

void StudentWorld::addProtester() {
    int level = getLevel();
    int ticksToAddProtester = max(25, 200 - level);
    int targetProtester = min(15, 2 + level*1.5);
    int probOfHardCore = min(90, level*10+30);
    int rand = randInt(1, 100);
    if (m_tick % ticksToAddProtester == 0 && m_numProtesters < targetProtester) {
        if (rand <= probOfHardCore)
            m_object.push_back(new Hard(60,60,this));
        else
            m_object.push_back(new Regular(60, 60, this));
        m_numProtesters++;
        
    }
}
