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

function move(scene)
    toZone(scene)
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