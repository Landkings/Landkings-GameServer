local env = {test = test, Scene = Scene}

local function run(untrusted_code)
    local untrusted_function, message = load(untrusted_code, nil, 't', env)
    if not untrusted_function then return nil, message end
    return pcall(untrusted_function)
end

local a = 10

local function inc(int)
    a = a + int
    return a
end

local function move(scene)
    scene:tist(a)
    inc(1)
end

run[[
--test()
--test(1, 0)
Scene:tist(4)
]]
--Scene:tist(4)