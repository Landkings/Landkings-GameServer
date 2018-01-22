function getDist(myPos, objPos)
    return ((myPos:getX() - objPos:getX())^2 + (myPos:getY() - objPos:getY())^2)^0.5;
end

function getNearest(objects)
    candidat = nil;
    minDistance = 9999999999;
    myPos = getPosition();
    for i = 1, #objects do
        objPos = objects[i]:getPosition();
        distance = getDist(myPos, objPos);
        if (minDistance > distance) then
            candidat = objects[i];
        end
    end
    return candidat;
end

function getFirstMoveDirection(xdiff, ydiff)
    if (math.abs(xdiff) - math.abs(ydiff) >= 0) then
        if (xdiff <= 0) then
            return Direction.Left;
        else
            return Direction.Right;
        end
    else
        if (ydiff <= 0) then
            return Direction.Up;
        else
            return Direction.Down;
        end
    end
end

function getSecondMoveDirection(first, xdiff, ydiff)
    if (first == Direction.Up or first == Direction.Down) then
        if (xdiff <= 0) then
            return Direction.Left;
        else
            return Direction.Right;
        end
    else
        if (ydiff <= 0) then
            return Direction.Up;
        else
            return Direction.Down;
        end
    end
end

function getThirdMoveDirection(first, second, xdiff, ydiff)
    if (math.abs(xdiff) - math.abs(ydiff) >= 0) then
        if (second == Direction.Up) then
            return Direction.Down;
        else
            return Directin.Up;
        end
    else
        if (first == Direction.Right) then
            return Direction.Left;
        else
            return Directin.Right;
        end
    end
end

function getOptMoveDirection(xdiff, ydiff)
    firstMoveDirection = nil;
    secondMoveDirection = nil;
    firstMoveDirection = getFirstMoveDirection(xdiff, ydiff);
    if (not canMove(firstMoveDirection)) then
        secondMoveDirection = getSecondMoveDirection(firstMoveDirection, xdiff, ydiff);
    else
        return firstMoveDirection;
    end
    if (not canMove(secondMoveDirection)) then
        return getThirdMoveDirection(firstMoveDirection, secondMoveDirection, xdiff, ydiff);
    else
        return secondMoveDirection;
    end
end

function move(scene)
    objects = scene:getObjects();
    if (#objects == 0) then
        setAction(Action.Empty);
        return;
    end
    setAction(Action.Move);
    nearest = getNearest(objects);
    npos = nearest:getPosition();
    mpos = getPosition();
    xdiff = mpos:getX() - npos:getX();
    ydiff = mpos:getY() - npos:getY();
    moveDirection = getOptMoveDirection(xdiff, ydiff);
    setDirection(moveDirection);
end

