#pragma once
#include <vector>

#include "lua.hpp"
#include "LuaHelper.h"

namespace Engine {

class Item;

class Inventory {
public:
    Inventory(int size) : maxSize(size), size(0) {}
    void addItem(Item *item);
    void updateMaxSize(int newMaxSize) { maxSize = newMaxSize; }
    void deleteUsedItems();
    bool canFit(Item *item);
    void luaPush(lua_State *state);
private:
    int luaGetItems(lua_State *state);

    int maxSize;
    int size;
    std::vector<Item*> items;
};
}
