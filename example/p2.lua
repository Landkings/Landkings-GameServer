steps = 0
function move(scene) 
    setAction(Action.Move)
  	a = math.random(4) / 1
  	if steps == 0 then
    	steps = 100
     if a == 1 then
      	setDirection(Direction.Right)
      elseif a == 2 then
      	setDirection(Direction.Left)
      elseif a == 3 then
      	setDirection(Direction.Up)
      else 
      	setDirection(Direction.Down)
     	end
    	if not canMoove(getDirection()) then
      	steps = 0
      end
    end
  steps = steps - 1
end