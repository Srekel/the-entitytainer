# the-entitytainer

A single header library for managing entity hierarchies

Basically a multimap implementation in C, aimed at game development.

Its main purpose is to keep track of hierarchies of entities. This can be useful for attachments (e.g. holding a weapon in
the hand) and inventory (having a piece of cheese in a bag in the backpack on the back of a character) for example.

The name is a pun of entities and containers. If it wasn't obvious.

## Problem statement

You want a one-to-many relationship but you don't want lots and lots of small array allocations.

For example, let's say you are trying to add support for characters in your game to have an inventory. Of course you want to do this by placing zero or more entities (items) into your inventory. Furiously, you start typing. Hours later you gaze upon your creation:

```C
typedef int Entity;

struct Inventory {
    Entity m_Entity;
    std::vector<Entity> m_ItemEntities;
};

struct InventorySystem {
    std::vector<Inventory> m_Inventories;
};
```

You realize that this is bad. And not just for using STL. But how to do it properly?

This is what The Entitytainer solves.

## Features

* It only does one allocation.
* A hierarchical bucket system is used to not waste memory.
* O(1) lookup, add, removal.
* Reverse lookup to get parent from a child.
* C99 compatible (or aims to be).
* No dependencies

## Current status

Builds but unit test doesn't even start.

## Known issues

* Only tested on Windows 10 using VS 2017.

## License

Dual licensed under Public Domain and MIT.
