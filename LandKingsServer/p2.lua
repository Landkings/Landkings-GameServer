function move(scene)
    --scene:tist(Action:Move)
    setAction(Action.Attack)
    setTarget(scene:getObjects()[1])
    --setDirection(Direction.Right)
    --objects = scene:getObjects()
    write(tostring(objects[2]))
end