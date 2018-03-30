function move(scene) 
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