#include "Actor.h"
#include "StudentWorld.h"
#include <string>
using namespace std;
// Actor
Actor::Actor(int imageID, int startX, int startY, StudentWorld* world, Direction startDirection, float size, unsigned int depth, string state)
: GraphObject(imageID, startX, startY, startDirection, size, depth), m_alive(true), m_world(world), m_state(state), m_tick(0)
{
}

Actor::~Actor()
{}

void Actor::annoy(int points)
{    
}

string Actor::getState() {
    return m_state;
}

void Actor::updateState(string newState) {
    m_state = newState;
}

bool Actor::alive() {
    return m_alive;
}

void Actor::setDead() {
    m_alive = false;
}

void Actor::incTick() {
    m_tick++;
}

void Actor::resetTick() {
    m_tick = 0;
}

int Actor::getTick() {
    return m_tick;
}

StudentWorld* Actor::getWorld() {
    return m_world;
}

// Dirt
Dirt::Dirt(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: Actor(imageID, startX, startY, world, startDirection, size, depth)
{
    setVisible(true);
}

Dirt::~Dirt()
{}

void Dirt::doSomething()
{
}

// Boulder
Boulder::Boulder(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: Actor(imageID, startX, startY, world, startDirection, size, depth, "stable")
{
    setVisible(true);
}

Boulder::~Boulder()
{}

void Boulder::doSomething() {
    if (!alive())
        return;
    int x = getX();
    int y = getY();
    if (getState() == "stable") {
        if (getWorld()->dirtExistsInBox(x, y-1, 4, 1)) {
            return;
        }
        updateState("waiting");
    }
    else if (getState() == "waiting") {
        if (getTick() == 30) {
            updateState("falling");
            GameController::getInstance().playSound(SOUND_FALLING_ROCK);
        }
        incTick();
    }
    else if (getState() == "falling") {
        if (!getWorld()->distToFracmanGreaterThanN(x, y, 3)) {
            getWorld()->getFrackMan()->annoy(-10);
        }
        if (getWorld()->objectAnnoyProtester(x, y, -10) || getWorld()->objectAnnoyHardProtester(x, y, -10)) {
            getWorld()->increaseScore(500);
        }
        if (y > 0 && !getWorld()->dirtExistsInBox(x, y-1, 4, 1) && getWorld()->distGreaterThanNSquares(x, y, 4, true, IID_BOULDER, true)) {
            moveTo(x, y - 1);
        }
        else {
            setDead();
        }
    }
}

// ActivatingObject
ActivatingObject::ActivatingObject(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth, string state)
: Actor(imageID, startX, startY, world, startDirection, size, depth, state)
{
    m_maxTick = getWorld()->max(100, 300 - 10 * getWorld()->getLevel());
}

ActivatingObject::~ActivatingObject()
{}

int ActivatingObject::getMaxTick() {
    return m_maxTick;
}

// Barrel
Barrel::Barrel(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: ActivatingObject(startX, startY, world, imageID, startDirection, size, depth)
{
}

Barrel::~Barrel()
{}

void Barrel::doSomething() {
    if (!alive())
        return;
    if (!isVisible() && !getWorld()->distToFracmanGreaterThanN(getX(), getY(), 4)) {
        setVisible(true);
        return;
    }
    else if (!getWorld()->distToFracmanGreaterThanN(getX(), getY(), 3)) {
        setDead();
        GameController::getInstance().playSound(SOUND_FOUND_OIL);
        getWorld()->increaseScore(1000);
    }
}

// Gold
Gold::Gold(int startX, int startY, StudentWorld* world, string state, int imageID, Direction startDirection, float size, unsigned int depth)
: ActivatingObject(startX, startY, world, imageID, startDirection, size, depth, state)
{
    if (getState() == "temporary")
        setVisible(true);
}

Gold::~Gold()
{}

void Gold::doSomething() {
    if (!alive())
        return;
    if (!isVisible() && !getWorld()->distToFracmanGreaterThanN(getX(), getY(), 4)) {
        setVisible(true);
        return;
    }
    else if (getState() == "permanent" && !getWorld()->distToFracmanGreaterThanN(getX(), getY(), 3)) {
        setDead();
        GameController::getInstance().playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(10);
        getWorld()->getFrackMan()->changeGold(1);
    }
    else if (getState() == "temporary") {
        Actor* protester = getWorld()->bribeProtester(getX(), getY());
        if (getTick() == 100) {
            setDead();
        }
        else if (protester != nullptr) {
            setDead();
            GameController::getInstance().playSound(SOUND_PROTESTER_FOUND_GOLD);
            if (protester->getID() == IID_PROTESTER) {
                getWorld()->increaseScore(25);
                protester->updateState("leave");
            }
            else if (protester->getID() == IID_HARD_CORE_PROTESTER) {
                getWorld()->increaseScore(50);
                protester->updateState("gold");
            }
        }
        incTick();
    }
}

// Sonar
Sonar::Sonar(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: ActivatingObject(startX, startY, world, imageID, startDirection, size, depth)
{
    setVisible(true);
}

Sonar::~Sonar()
{}

void Sonar::doSomething() {
    if (!alive())
        return;
    if (!getWorld()->distToFracmanGreaterThanN(getX(), getY(), 3)) {
        setDead();
        GameController::getInstance().playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(75);
        getWorld()->getFrackMan()->changeSonar(1);
    }
    else if (getTick() == getMaxTick()) {
        setDead();
    }
    incTick();
}

// Water
Water::Water(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: ActivatingObject(startX, startY, world, imageID, startDirection, size, depth)
{
    setVisible(true);
}

Water::~Water()
{}

void Water::doSomething() {
    if (!alive())
        return;
    if (!getWorld()->distToFracmanGreaterThanN(getX(), getY(), 3)) {
        setDead();
        GameController::getInstance().playSound(SOUND_GOT_GOODIE);
        getWorld()->increaseScore(100);
        getWorld()->getFrackMan()->changeWater(5);
    }
    else if (getTick() == getMaxTick()) {
        setDead();
    }
    incTick();
}

// Squirt
Squirt::Squirt(int startX, int startY, StudentWorld* world, Direction startDirection, int imageID, float size, unsigned int depth)
: Actor(imageID, startX, startY, world, startDirection, size, depth), m_distance(0)
{
    setVisible(true);
}

Squirt::~Squirt()
{}

void Squirt::doSomething() {
    int x = getX();
    int y = getY();
    if (m_distance < 4) {
        if (getWorld()->objectAnnoyHardProtester(x, y, -2)) {
            getWorld()->increaseScore(250);
            setDead();
        }
        else if (getWorld()->objectAnnoyProtester(x, y, -2)) {
            getWorld()->increaseScore(100);
            setDead();
        }
        else if (getDirection() == left) {
            if (x > 0 && !getWorld()->dirtExistsInBox(x, y, 1, 4) && !getWorld()->boulderExistsInBox(x, y, 1, 4)) {
                moveTo(x-1, y);
            }
            else {
                setDead();
            }
        }
        else if (getDirection() == right) {
            if (x < 60 && !getWorld()->dirtExistsInBox(x+3, y, 1, 4) && !getWorld()->boulderExistsInBox(x+3, y, 1, 4)) {
                moveTo(x+1, y);
            }
            else {
                setDead();
            }
        }
        else if (getDirection() == up) {
            if (y < 60 && !getWorld()->dirtExistsInBox(x, y+3, 4, 1) && !getWorld()->boulderExistsInBox(x, y+3, 4, 1)) {
                moveTo(x, y+1);
            }
            else {
                setDead();
            }
        }
        else if (getDirection () == down) {
            if (y > 0 && !getWorld()->dirtExistsInBox(x, y, 4, 1) && !getWorld()->boulderExistsInBox(x, y, 4, 1)) {
                moveTo(x, y - 1);
            }
            else {
                setDead();
            }
        }
        m_distance++;
    }
    else {
        setDead();
    }
}

// FrackMan
FrackMan::FrackMan(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: Actor(imageID, startX, startY, world, startDirection, size, depth), m_hp(10), m_water(5), m_sonar(1), m_gold(0)
{
    setVisible(true);
}

FrackMan::~FrackMan()
{
}

void FrackMan::doSomething() {
    if (!alive())
        return;
    int x = getX();
    int y = getY();
    bool destroy_dirt = getWorld()->deleteDirt(x, y);
    if (destroy_dirt)
        GameController::getInstance().playSound(SOUND_DIG);
    int ch;
    if (getWorld()->getKey(ch) == true)
    {
        // user hit a key this tick!
        int x = getX();
        int y = getY();
        switch (ch)
        {
            case KEY_PRESS_ESCAPE:
                m_hp = 0;
                setDead();
                getWorld()->decLives();
                break;
            case KEY_PRESS_LEFT:
                if (getDirection() != left) {
                    setDirection(left);
                    moveTo(x, y);
                }
                else {
                    if (x > 0) {
                        if (getWorld()->distGreaterThanNSquares(x-1, y, 3, true, IID_BOULDER, false))
                            moveTo(x-1, y);
                    }
                    else
                        moveTo(x, y);
                }
                break;
            case KEY_PRESS_RIGHT:
                if (getDirection() != right) {
                    setDirection(right);
                    moveTo(x, y);
                }
                else {
                    if (x < 60) {
                        if (getWorld()->distGreaterThanNSquares(x+1, y, 3, true, IID_BOULDER, false))
                            moveTo(x+1, y);
                    }
                    else
                        moveTo(x, y);
                }
                break;
            case KEY_PRESS_UP:
                if (getDirection() != up) {
                    setDirection(up);
                    moveTo(x, y);
                }
                else {
                    if (y < 60) {
                        if (getWorld()->distGreaterThanNSquares(x, y+1, 3, true, IID_BOULDER, false))
                            moveTo(x, y+1);
                    }
                    else
                        moveTo(x, y);
                }
                break;
            case KEY_PRESS_DOWN:
                if (getDirection() != down) {
                    setDirection(down);
                    moveTo(x, y);
                }
                else {
                    if (y > 0) {
                        if (getWorld()->distGreaterThanNSquares(x, y-1, 3, true, IID_BOULDER, false))
                            moveTo(x, y-1);
                    }
                    else
                        moveTo(x, y);
                }
                break;
            case 'z':
            case 'Z':
                if (getSonar() > 0) {
                    GameController::getInstance().playSound(SOUND_SONAR);
                    getWorld()->setVisibleWithinDistanceN(12);
                    changeSonar(-1);
                }
                break;
            case KEY_PRESS_TAB:
                if (getGold() > 0) {
                    getWorld()->getObjects()->push_back(new Gold(x, y, getWorld(), "temporary"));
                    changeGold(-1);
                }
                break;
            case KEY_PRESS_SPACE:
                if (getWater() > 0) {
                    GameController::getInstance().playSound(SOUND_PLAYER_SQUIRT);
                    int startX = x;
                    int startY = y;
                    int checkX = x;
                    int checkY = y;
                    int dx = 0;
                    int dy = 0;
                    if (getDirection() == right) {
                        startX = startX + 4;
                        checkX = startX + 3;
                        dx = 1;
                        dy = 4;
                    }
                    else if (getDirection() == left) {
                        startX = startX - 4;
                        checkX = startX;
                        dx = 1;
                        dy = 4;
                    }
                    else if (getDirection() == down) {
                        startY = startY - 4;
                        checkY = startY;
                        dx = 4;
                        dy = 1;
                    }
                    else if (getDirection() == up) {
                        startY = startY + 4;
                        checkY = startY + 3;
                        dx = 4;
                        dy = 1;
                    }
                    if (!getWorld()->dirtExistsInBox(checkX, checkY, dx, dy) && !getWorld()->boulderExistsInBox(checkX, checkY, dx, dy)) {
                        getWorld()->getObjects()->push_back(new Squirt(startX, startY, getWorld(), getDirection()));
                    }
                    changeWater(-1);
                }
                break;
        }
    }
}

void FrackMan::annoy(int points) {
    changeHp(points);
    if (getHp() <= 0) {
        GameController::getInstance().playSound(SOUND_PLAYER_GIVE_UP);
        setDead();
    }
}

int FrackMan::getHp() {
    return m_hp;
}

int FrackMan::getWater() {
    return m_water;
}

int FrackMan::getSonar() {
    return m_sonar;
}

int FrackMan::getGold() {
    return m_gold;
}

void FrackMan::changeHp(int change) {
    m_hp += change;
}

void FrackMan::changeWater(int change) {
    m_water += change;
}

void FrackMan::changeSonar(int change) {
    m_sonar += change;
}

void FrackMan::changeGold(int change) {
    m_gold += change;
}

// Protester

Protester::Protester(int startX, int startY, StudentWorld* world, int hp, int imageID, Direction startDirection, float size, unsigned int depth)
: Actor(imageID, startX, startY, world, startDirection, size, depth), m_hp(hp)
{
}

Protester::~Protester()
{}

int Protester::getHp() {
    return m_hp;
}

void Protester::changeHp(int change) {
    m_hp += change;
}

bool Protester::facingFrackMan(int x, int y, Direction direction) {
    int fx = getWorld()->getFrackMan()->getX();
    int fy = getWorld()->getFrackMan()->getY();
    if (direction == left) {
        if (fx <= x)
            return true;
    }
    else if (direction == right) {
        if (fx >= x)
            return true;
    }
    else if (direction == down) {
        if (fy <= y)
            return true;
    }
    else if (direction == up) {
        if (fy >= y)
            return true;
    }
    return false;
}

bool Protester::inSight(int x, int y) {
    int fx = getWorld()->getFrackMan()->getX();
    int fy = getWorld()->getFrackMan()->getY();
    StudentWorld* world = getWorld();
    if (x == fx && world->distToFracmanGreaterThanN(x, y, 4) && !world->dirtExistsInBox(x, world->min(y, fy), 4, world->abs(y-fy)) && !world->boulderExistsInBox(x, world->min(y, fy), 4, world->abs(y-fy))) {
        return true;
    }
    if (y == fy && world->distToFracmanGreaterThanN(x, y, 4) && !world->dirtExistsInBox(world->min(x,fx), y, world->abs(x-fx), 4) && !world->boulderExistsInBox(world->min(x,fx), y, world->abs(x-fx), 4)) {
        return true;
    }
    return false;
}

Actor::Direction Protester::pickNewDirection() {
    int n = getWorld()->randInt(1,4);
    if (n == 1)
        return up;
    else if (n == 2)
        return down;
    else if (n == 3)
        return left;
    else
        return right;
}

void Protester::checkDirtBoulder(int& x, int& y, int& dx, int& dy, Direction start) {
    if (start == left) {
        x = x - 1;
        dx = 1;
        dy = 4;
    }
    else if (start == right) {
        x = x + 4;
        dx = 1;
        dy = 4;
    }
    else if (start == down) {
        y = y - 1;
        dx = 4;
        dy = 1;
    }
    else if (start == up) {
        y = y + 4;
        dx = 4;
        dy = 1;
    }
}

Actor::Direction Protester::chooseNotBlockedDirection(Direction start, int x, int y) {
    int dx;
    int dy;
    int tempX = x;
    int tempY = y;
    checkDirtBoulder(x, y, dx, dy, start);
    while (getWorld()->dirtExistsInBox(x, y, dx, dy) || getWorld()->boulderExistsInBox(x, y, dx, dy) || y < 0 || y > 63 || x < 0 || x > 63) {
        start = pickNewDirection();
        x = tempX;
        y = tempY;
        checkDirtBoulder(x, y, dx, dy, start);
    }
    return start;
}

Actor::Direction Protester::makePerpendicularTurn(int x, int y, Direction current) {
    bool one = false;
    bool two = false;
    bool three = false;
    bool four = false;
    StudentWorld* world = getWorld();
    if (current == left || current == right) {
        if (!world->dirtExistsInBox(x, y-1, 4, 1) && !world->boulderExistsInBox(x, y-1, 4, 1) && y > 0)
            one = true;
        if (!world->dirtExistsInBox(x, y+4, 4, 1) && !world->boulderExistsInBox(x, y+4, 4, 1) && y < 60)
            two = true;
    }
    else if (current == up || current == down) {
        if (!world->dirtExistsInBox(x-1, y, 1, 4) && !world->boulderExistsInBox(x-1, y, 1, 4) && x > 0)
            three = true;
        if (!world->dirtExistsInBox(x+4, y, 1, 4) && !world->boulderExistsInBox(x+4, y, 1, 4) && x < 60)
            four = true;
    }
    if (one || two) {
        if (one && two) {
            int n = world->randInt(0,1);
            if (n == 0)
                return up;
            return down;
        }
        else if (one) {
            return down;
        }
        else {
            return up;
        }
    }
    else {
        if (three && four) {
            int n = world->randInt(0,1);
            if (n == 0)
                return right;
            return left;
        }
        else if (three) {
            return left;
        }
        else {
            return right;
        }
    }
}

bool Protester::canMakeTurn(int x, int y, Direction current) {
    StudentWorld* world = getWorld();
    if (current == left || current == right) {
        if (!world->dirtExistsInBox(x, y-1, 4, 1) && !world->boulderExistsInBox(x, y-1, 4, 1) && y > 0)
            return true;
        if (!world->dirtExistsInBox(x, y+4, 4, 1) && !world->boulderExistsInBox(x, y+4, 4, 1) && y < 60)
            return true;
    }
    else if (current == up || current == down) {
        if (!world->dirtExistsInBox(x-1, y, 1, 4) && !world->boulderExistsInBox(x-1, y, 1, 4) && x > 0)
            return true;
        if (!world->dirtExistsInBox(x+4, y, 1, 4) && !world->boulderExistsInBox(x+4, y, 1, 4) && x < 60)
            return true;
    }
    return false;
}

// Regular
Regular::Regular(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: Protester(startX, startY, world, 5, imageID, startDirection, size, depth), m_canShout(true), m_ticksToShout(0), m_canTurn(false), m_ticksToTurn(0), m_stun(false), m_ticksToUnstun(0)
{
    m_ticksToWaitBetweenMoves = getWorld()->max(0, 3 - getWorld()->getLevel()/4);
    m_numSquaresToMoveInCurrentDirection = getWorld()->randInt(8, 60);
    m_restingTicks = getWorld()->max(50, 100 - getWorld()->getLevel()*10);
    setVisible(true);
}

Regular::~Regular()
{
}

void Regular::doSomething() {
    if (!alive())
        return;
    int x = getX();
    int y = getY();
    if (m_stun) {
        if (m_ticksToUnstun == m_restingTicks) {
            m_ticksToUnstun = 0;
            m_stun = false;
        }
        else {
            m_ticksToUnstun++;
        }
        return;
    }
    if (getTick() % m_ticksToWaitBetweenMoves == 0) {
        if (!m_canShout) {
        }
        else if (getState() == "leave") {
            if (x == 60 && y == 60) {
                getWorld()->decNumProtesters();
                setDead();
            }
            else {
                setDirection(getWorld()->getOptimalPath(x, y));
                Direction togo = getDirection();
                if (togo == left) {
                    moveTo(x-1, y);
                }
                else if (togo == right) {
                    moveTo(x+1, y);
                }
                else if (togo == up) {
                    moveTo(x, y+1);
                }
                else if (togo == down) {
                    moveTo(x, y-1);
                }
            }
        }
        else if (!getWorld()->distToFracmanGreaterThanN(x, y, 4) && m_canShout && facingFrackMan(x, y, getDirection())) {
            GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
            getWorld()->getFrackMan()->annoy(-2);
            m_ticksToShout = 0;
            m_canShout = false;
        }
        else if (inSight(x, y)) {
            int fx = getWorld()->getFrackMan()->getX();
            int fy = getWorld()->getFrackMan()->getY();
            if (fx < x) {
                setDirection(left);
                moveTo(x-1, y);
            }
            else if (fx > x) {
                setDirection(right);
                moveTo(x+1, y);
            }
            else if (fy < y) {
                setDirection(down);
                moveTo(x, y-1);
            }
            else if (fy > y) {
                setDirection(up);
                moveTo(x, y+1);
            }
            m_numSquaresToMoveInCurrentDirection = 0;
        }
        else {
            m_numSquaresToMoveInCurrentDirection--;
            if (m_numSquaresToMoveInCurrentDirection <= 0) {
                Direction start = pickNewDirection();
                setDirection(chooseNotBlockedDirection(start, x, y));
                m_numSquaresToMoveInCurrentDirection = getWorld()->randInt(8, 60);
            }
            else {
                if (m_canTurn && canMakeTurn(x, y, getDirection())) {
                    setDirection(makePerpendicularTurn(x, y, getDirection()));
                    m_numSquaresToMoveInCurrentDirection = getWorld()->randInt(8, 60);
                    m_ticksToTurn = 0;
                    m_canTurn = false;
                }
            }
            if (getDirection() == left && x > 0 && !getWorld()->dirtExistsInBox(x-1, y, 1, 4) && !getWorld()->boulderExistsInBox(x-1, y, 1, 4)) {
                moveTo(x-1, y);
            }
            else if (getDirection() == right && x < 60 && !getWorld()->dirtExistsInBox(x+4, y, 1, 4) && !getWorld()->boulderExistsInBox(x+4, y, 1, 4)) {
                moveTo(x+1, y);
            }
            else if (getDirection() == down && y > 0 && !getWorld()->dirtExistsInBox(x, y-1, 4, 1) && !getWorld()->boulderExistsInBox(x, y-1, 4, 1)) {
                moveTo(x, y-1);
            }
            else if (getDirection() == up && y < 60 && !getWorld()->dirtExistsInBox(x, y+4, 4, 1) && !getWorld()->boulderExistsInBox(x, y+4, 4, 1)) {
                moveTo(x, y+1);
            }
            else {
                m_numSquaresToMoveInCurrentDirection = 0;
            }
        }
        m_ticksToShout++;
        m_ticksToTurn++;
        if (!m_canShout && m_ticksToShout % 15 == 0) {
            m_canShout = true;
        }
        if (!m_canTurn && m_ticksToTurn % 200 == 0) {
            m_canTurn = true;
        }
        
    }
    incTick();
}

void Regular::annoy(int points) {
    changeHp(points);
    if (getHp() <= 0) {
        getWorld()->callFindOptimalPath();
        GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
        updateState("leave");
        m_stun = false;
        resetTick();
    }
    else {
        GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
        m_stun = true;
        resetTick();
    }
}
// Hard

Hard::Hard(int startX, int startY, StudentWorld* world, int imageID, Direction startDirection, float size, unsigned int depth)
: Protester(startX, startY, world, 20, imageID, startDirection, size, depth), m_canShout(true), m_ticksToShout(0), m_canTurn(false), m_ticksToTurn(0), m_stun(false), m_ticksToUnstun(0)
{
    m_ticksToWaitBetweenMoves = getWorld()->max(0, 3 - getWorld()->getLevel()/4);
    m_numSquaresToMoveInCurrentDirection = getWorld()->randInt(8, 60);
    m_restingTicks = getWorld()->max(50, 100 - getWorld()->getLevel()*10);
    setVisible(true);
}

Hard::~Hard()
{}

void Hard::doSomething() {
    if (!alive())
        return;
    int x = getX();
    int y = getY();
    Direction toFrackMan = getWorld()->getPathToFrackMan(x, y);
    if (m_stun || getState() == "gold") {
        if (m_ticksToUnstun == m_restingTicks) {
            m_ticksToUnstun = 0;
            m_stun = false;
            updateState("");
        }
        else {
            m_ticksToUnstun++;
        }
        return;
    }
    if (getTick() % m_ticksToWaitBetweenMoves == 0) {
        if (!m_canShout) {
        }
        else if (getState() == "leave") {
            if (x == 60 && y == 60) {
                getWorld()->decNumProtesters();
                setDead();
            }
            else {
                setDirection(getWorld()->getOptimalPath(x, y));
                Direction togo = getDirection();
                if (togo == left) {
                    moveTo(x-1, y);
                }
                else if (togo == right) {
                    moveTo(x+1, y);
                }
                else if (togo == up) {
                    moveTo(x, y+1);
                }
                else if (togo == down) {
                    moveTo(x, y-1);
                }
            }
        }
        else if (!getWorld()->distToFracmanGreaterThanN(x, y, 4) && m_canShout && facingFrackMan(x, y, getDirection())) {
            GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
            getWorld()->getFrackMan()->annoy(-2);
            m_ticksToShout = 0;
            m_canShout = false;
        }
        else if (getWorld()->distToFracmanGreaterThanN(x, y, 4) && toFrackMan != none) {
            setDirection(toFrackMan);
            Direction togo = getDirection();
            if (togo == left) {
                moveTo(x-1, y);
            }
            else if (togo == right) {
                moveTo(x+1, y);
            }
            else if (togo == up) {
                moveTo(x, y+1);
            }
            else if (togo == down) {
                moveTo(x, y-1);
            }
        }
        else if (inSight(x, y)) {
            int fx = getWorld()->getFrackMan()->getX();
            int fy = getWorld()->getFrackMan()->getY();
            if (fx < x) {
                setDirection(left);
                moveTo(x-1, y);
            }
            else if (fx > x) {
                setDirection(right);
                moveTo(x+1, y);
            }
            else if (fy < y) {
                setDirection(down);
                moveTo(x, y-1);
            }
            else if (fy > y) {
                setDirection(up);
                moveTo(x, y+1);
            }
            m_numSquaresToMoveInCurrentDirection = 0;
        }
        else {
            m_numSquaresToMoveInCurrentDirection--;
            if (m_numSquaresToMoveInCurrentDirection <= 0) {
                Direction start = pickNewDirection();
                setDirection(chooseNotBlockedDirection(start, x, y));
                m_numSquaresToMoveInCurrentDirection = getWorld()->randInt(8, 60);
            }
            else {
                if (m_canTurn && canMakeTurn(x, y, getDirection())) {
                    setDirection(makePerpendicularTurn(x, y, getDirection()));
                    m_numSquaresToMoveInCurrentDirection = getWorld()->randInt(8, 60);
                    m_ticksToTurn = 0;
                    m_canTurn = false;
                }
            }
            if (getDirection() == left && x > 0 && !getWorld()->dirtExistsInBox(x-1, y, 1, 4) && !getWorld()->boulderExistsInBox(x-1, y, 1, 4)) {
                moveTo(x-1, y);
            }
            else if (getDirection() == right && x < 60 && !getWorld()->dirtExistsInBox(x+4, y, 1, 4) && !getWorld()->boulderExistsInBox(x+4, y, 1, 4)) {
                moveTo(x+1, y);
            }
            else if (getDirection() == down && y > 0 && !getWorld()->dirtExistsInBox(x, y-1, 4, 1) && !getWorld()->boulderExistsInBox(x, y-1, 4, 1)) {
                moveTo(x, y-1);
            }
            else if (getDirection() == up && y < 60 && !getWorld()->dirtExistsInBox(x, y+4, 4, 1) && !getWorld()->boulderExistsInBox(x, y+4, 4, 1)) {
                moveTo(x, y+1);
            }
            else {
                m_numSquaresToMoveInCurrentDirection = 0;
            }
        }
        m_ticksToShout++;
        m_ticksToTurn++;
        if (!m_canShout && m_ticksToShout % 15 == 0) {
            m_canShout = true;
        }
        if (!m_canTurn && m_ticksToTurn % 200 == 0) {
            m_canTurn = true;
        }
        
    }
    incTick();

}

void Hard::annoy(int points) {
    changeHp(points);
    if (getHp() <= 0) {
        getWorld()->callFindOptimalPath();
        GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
        updateState("leave");
        m_stun = false;
        resetTick();
    }
    else {
        GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
        m_stun = true;
        resetTick();
    }
}