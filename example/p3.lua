intDir = { [0] = Direction.Up, [1] = Direction.Right, [2] = Direction.Down, [3] = Direction.Left }
dirInt = { [Direction.Up] = 0, [Direction.Right] = 1, [Direction.Down] = 2, [Direction.Left] = 3 }
countChangeDir = 0

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

function dist(pos1, pos2)
    return math.abs(pos1:getX() - pos2:getX()) + math.abs(pos1:getY() - pos2:getY())
end

function toZone(scene)
    setAction(Action.Move)
    dirx = 1
    diry = 1
    if scene:getSafeZone():getPosition():getX() == getMe():getPosition():getX() and
            scene:getSafeZone():getPosition():getY() == getMe():getPosition():getY() then
        setAction(Action.Empty)
    end
    if scene:getSafeZone():getPosition():getX() - getMe():getPosition():getX() < 50 then
        dirx = Direction.Left
    elseif scene:getSafeZone():getPosition():getX() - getMe():getPosition():getX() > 50 then
        dirx = Direction.Right
    end
    if scene:getSafeZone():getPosition():getY() - getMe():getPosition():getY() < 50 then
        diry = Direction.Up
    elseif scene:getSafeZone():getPosition():getY() - getMe():getPosition():getY() > 50 then
        diry = Direction.Down
    end
    chooseDir(dirx, diry)
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
    setTarget(getMe())
    setMovementType(MovementType.Default)
    toZone(scene)
    objects = scene:getObjects();
    min = 10000
    for i = 1, #objects do
        pos1 = getPosition()
        pos2 = objects[i]:getPosition()
        d = dist(pos1, pos2)
        if d < min then
            if objects[i]:getObjectType() == ObjectType.NPC then
                min = d
                goAway(pos1, pos2)
                runIfNeed(pos1, pos2)
            elseif objects[i]:getObjectType() == ObjectType.Player then
                min = d
                setAction(Action.Attack)
                setAttackDirection(AttackDirection.Torso)
                setTarget(objects[i])
            end
        end
    end
end