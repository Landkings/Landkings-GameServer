intDir = { [0] = Direction.Up, [1] = Direction.Right, [2] = Direction.Down, [3] = Direction.Left }
dirInt = { [Direction.Up] = 0, [Direction.Right] = 1, [Direction.Down] = 2, [Direction.Left] = 3 }
countChangeDir = 0
dmgLVL = 0
curZoneX = -10000
curZoneY = -10000


function setdir(dir)
    countChangeDir = countChangeDir + 1
    if canMove(dir) == false and countChangeDir < 5 then
        dir = (dir + 1) % 4
        setdir(dir)
        return
    end
    countChangeDir = 0
    setDirection(intDir[dir])
end

function setDir(dir)
    setdir(dirInt[dir])
end

function chooseDir(dirx, diry)
    if math.random(2) == 2 then
        setDir(dirx)
    else
        setDir(diry)
    end
end

function dist(pos1, pos2)
    return math.abs(pos1:getX() - pos2:getX()) + math.abs(pos1:getY() - pos2:getY())
end


function goAway(pos1, pos2)
    setAction(Action.Move)
    if math.abs(pos1:getX() - pos2:getX()) > math.abs(pos1:getY() - pos2:getY()) then
        if pos1:getX() > pos2:getX() then
            setDir(Direction.Right)
        else
            setDir(Direction.Left)
        end
    else
        if pos1:getY() > pos2:getY() then
            setDir(Direction.Down)
        else
            setDir(Direction.Up)
        end
    end
end

function moveToPoint(x, y, delta)
    setAction(Action.Move)
    dirX = 1
    dirY = 1
    meX = getPosition():getX()
    meY = getPosition():getY()
    if x == meX and y == meY then
        setAction(Action.Empty)
        return
    end
    if x - meX < delta then
        dirX = Direction.Left
    elseif x - meX > delta then
        dirX = Direction.Right
    end
    if y - meY < delta then
        dirY = Direction.Up
    elseif y - meY > delta then
        dirY = Direction.Down
    end
    chooseDir(dirX, dirY)
end

function toZone(scene)
    zoneX = scene:getSafeZone():getPosition():getX()
    zoneY = scene:getSafeZone():getPosition():getY()
    radius = scene:getSafeZone():getRadius()
    x = curZoneX
    y = curZoneY
    if curZoneX < zoneX - radius or curZoneX > zoneX + radius then
        x = math.random(zoneX - radius, zoneX + radius)
    end
    if curZoneY < zoneY - radius or curZoneY > zoneY + radius then
        y = math.random(zoneY - radius, zoneY + radius)
    end
    if curZoneX == getPosition():getX() and curZoneY == getPosition():getY() then
        x = math.random(zoneX - radius, zoneX + radius)
        y = math.random(zoneY - radius, zoneY + radius)
    end
    curZoneX = x
    curZoneY = y
    moveToPoint(x, y, 0)
end

function runIfNeed(pos1, pos2)
    if math.sqrt((pos1:getX() - pos2:getX()) ^ 2 + (pos1:getY() - pos2:getY()) ^ 2) < 110 then
        sprintIfCan()
    end
end

function sprintIfCan()
    setMovementType(MovementType.Default)
    if getStamina() > getSprintStaminaCost() then
        setMovementType(MovementType.Sprint)
    end
end

function move(scene)
    if getParameterLevelUpCost(Parameters.AttackDamage) < getAvailableSkillPoints() then
    levelUp(Parameters.AttackDamage) 
    dmgLVL = dmgLVL + 1
    end
    setTarget(getMe())
    setMovementType(MovementType.Default)
    toZone(scene)
    objects = scene:getObjects();
    min = 10000
    npcCount = 0
    for i = 1, #objects do
        pos1 = getPosition()
        pos2 = objects[i]:getPosition()
        d = dist(pos1, pos2)
        if d < min then
            if objects[i]:getObjectType() == ObjectType.NPC then
                npcCount = npcCount + 1
                min = d
                if npcCount > 2 then
                    goAway(pos1, pos2)
                    runIfNeed(pos1, pos2)
                else
                    setAction(Action.Attack)
                    setAttackDirection(AttackDirection.Torso)
                    setTarget(objects[i])
                end
            elseif objects[i]:getObjectType() == ObjectType.Player and dmgLVL > 0 then
                min = d
                setAction(Action.Attack)
                setAttackDirection(AttackDirection.Torso)
                setTarget(objects[i])
            else
                goAway(pos1, pos2)
                runIfNeed(pos1, pos2)
            end
        end
    end
end