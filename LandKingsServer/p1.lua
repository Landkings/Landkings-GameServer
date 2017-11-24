a = 0
function move(scene)
    --scene:tist(Action:Move)
    if a < 10 then
        setAction(Action.Move)
        setDirection(Direction.Right)
        a = a + 1
    elseif a == 10 then
        setAction(Action.Empty)
    end
end