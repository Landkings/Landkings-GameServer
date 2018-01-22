function getDist(myPos, objPos)
    return ((myPos:getX() - objPos:getX())^2 + (myPos:getY() - objPos:getY())^2)^0.5;
end

function getNearest(objects)
    candidat = nil;
    minDistance = 9999999999;
    myPos = getMe():getPosition();
    for i = 1, #objects do
        objPos = objects[i]:getPosition();
        distance = getDist(myPos, objPos);
        if (minDistance > distance) then
            candidat = objects[i];
        end
    end
    return candidat;
end

function move(scene)
    objects = scene:getObjects()
    if (#objects > 0) then
        target = getNearest(objects);
        setTarget(target);
        setAction(Action.Attack);
        setAttackType(AttackType.Strong);
        setAttackDirection(AttackDirection.Head);
    else
        setAction(Action.Empty);
    end
end