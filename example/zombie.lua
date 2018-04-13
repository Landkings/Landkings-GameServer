intDir = { [0] = Direction.Up, [1] = Direction.Right, [2] = Direction.Down, [3] = Direction.Left }
dirInt = { [Direction.Up] = 0, [Direction.Right] = 1, [Direction.Down] = 2, [Direction.Left] = 3 }
countChangeDir = 0
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

function moveToZone(scene)
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

function move(scene)
    setTarget(getMe())
    moveToZone(scene)
    setMovementType(MovementType.Default)
    objects = scene:getObjects()
    min = 1000000
    for i = 1, #objects do
        dist(getPosition(), objects[i]:getPosition())
        if objects[i]:getObjectType() == ObjectType.Player then
            setAction(Action.Attack)
            setAttackDirection(AttackDirection.Torso)
            setMovementType(MovementType.Sprint)
            d = dist(getPosition(), objects[i]:getPosition())
            if d < min then
                min = d
                setTarget(objects[i])
            end
        end
    end
end