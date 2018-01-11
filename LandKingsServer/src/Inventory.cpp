#include "Inventory.h"
#include "GameObject.h"

using namespace Engine;

void Inventory::addItem(Item *item) {
    int newSize = size + item->getSize();
    if (newSize <= maxSize) {
        size = newSize;
        items.push_back(item);
    }
}

void Engine::Inventory::deleteUsedItems() {
    for (int i = items.size(); i >= 0; --i) {
        if (items[i]->used()) {
            size -= items[i]->getSize();
            delete items[i];
            items.erase(items.begin() + i);
        }
    }
}

bool Inventory::canFit(Item *item) {
    return size + item->getSize() <= maxSize;
}

void Inventory::luaPush(lua_State *state) {
    Inventory **Pinv = (Inventory**)lua_newuserdata(state, sizeof(Inventory*));
    *Pinv = this;
    if (luaL_newmetatable(state, "InventoryMetaTable")) {
        lua_pushvalue(state, -1);
        lua_setfield(state, -2, "__index");

        luaL_Reg InventoryMethods[] = {
            "getItems", dispatch<Inventory, &Inventory::luaGetItems>,
            //"test", dispatch<Inventory, &Inventory::test>,
            nullptr, nullptr
        };
        luaL_setfuncs(state, InventoryMethods, 0);
    }
    lua_setmetatable(state, -2);
}

// private methods

int Inventory::luaGetItems(lua_State *state) {
    lua_newtable(state);
    int i = 0;
    for (auto& item: items) {
        item->luaPush(state);
        lua_rawseti(state, -2, ++i);
    }
    return 1;
}
