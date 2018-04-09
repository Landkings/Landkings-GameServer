function goAway(pos1, pos2)
    setAction(Action.Move)
    if math.abs(pos1:getX() - pos2:getX()) > math.abs(pos1:getY() - pos2:getY()) then
        if pos1:getX() > pos2:getX() then
            setDirection(Direction.Right)
        else
            setDirection(Direction.Left)
        end
    else
        if pos1:getY() > pos2:getY() then
            setDirection(Direction.Down)
        else
            setDirection(Direction.Up)
        end
    end
end

function dist(pos1, pos2)
    return math.abs(pos1:getX() - pos2:getX()) + math.abs(pos1:getY() - pos2:getY())
end

function toZone(scene)
    setAction(Action.Move)
    if scene:getSafeZone():getPosition():getX() == getMe():getPosition():getX() and
            scene:getSafeZone():getPosition():getY() == getMe():getPosition():getY() then
        setAction(Action.Empty)
    elseif scene:getSafeZone():getPosition():getX() < getMe():getPosition():getX() then
        setDirection(Direction.Left)
    elseif scene:getSafeZone():getPosition():getX() > getMe():getPosition():getX() then
        setDirection(Direction.Right)
    elseif scene:getSafeZone():getPosition():getY() < getMe():getPosition():getY() then
        setDirection(Direction.Up)
    elseif scene:getSafeZone():getPosition():getY() > getMe():getPosition():getY() then
        setDirection(Direction.Down)
    end
end

function runIfNeed(pos1, pos2)
    if math.sqrt((pos1:getX() - pos2:getX()) ^ 2 + (pos1:getY() - pos2:getY()) ^ 2) < 100 then
        if getStamina() > getSprintStaminaCost() then
            setMovementType(MovementType.Sprint)
        end
    end
end

function move(scene)
    setMovementType(MovementType.Default)
    toZone(scene)
    objects = scene:getObjects();
    min = 10000
    for i = 1, #objects do
        if objects[i]:getObjectType() == ObjectType.NPC then
            pos1 = getPosition()
            pos2 = objects[i]:getPosition()
            if dist(pos1, pos2) < min then
                goAway(pos1, pos2)
                runIfNeed(pos1, pos2)
            end
        end
    end
end