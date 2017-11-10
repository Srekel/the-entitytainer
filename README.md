# The Entitytainer

A single header library for managing entity hierarchies.

Basically a multimap implementation in C, aimed at game development.

Its main purpose is to keep track of hierarchies of entities. This can be useful for attachments (e.g. holding a weapon in
the hand) and inventory (having a piece of cheese in a bag in the backpack on the back of a character) for example.

The name is a pun of entities and containers. If it wasn't obvious. Credit for this amazing name goes to @fzetterman from this thread: https://twitter.com/Srekel/status/919845253032660993

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
* Can be dynamically reallocated - controlled by application.
* A hierarchical bucket system is used to not waste memory.
* O(1) lookup, add, removal.
* Reverse lookup to get parent from a child.
* C99 compatible (or aims to be).
* Zero dependencies.


## Current status

Builds but unit test doesn't even start.

## Known issues

* Only tested on Windows 10 using VS 2017.

## How it works

This image describes it at a high level.

![A graph that's... colorful and complicated.](docs/visual_explanation.png "Logo Title Text 1")

First you decide how many *entries* you want. This is be your maximum entity count. Note, it's NOT the maximum amount of entities you will maximally put into the entitytainer. At some point I might fix hashing but for now it's just a direct lookup based on the entity ID.

This number is used to create an array of *entries*. An entry is a 16 bit value that contains of two parts: The bucket list lookup and the bucket index.

The *list lookup* is 2 bits and shows which *bucket list* the entity's children are stored in. In the image example, **Entity 2**'s children are stored in the second bucket list (index 1).

The *bucket index* says which bucket in the bucket list the children are stored at. To look up the children of an entity, you first get the bucket list, and then the bucket inside that list.

The bucket consists of a number of elements. The first one is a *counter*, saying how many children there are. This is a simple way of storing the size so The Entitytainer doesn't has to iterate every time to figure it out. It's a bit wasteful in memory but it makes things a bit easier.

After the *counter* comes the children.

Each bucket list has buckets of different sizes. When a child is added to an entity and the bucket is full, the bucket is copied to a new bucket in the next bucket list. Note that you probably don't want your first bucket list to have bucket size 2, like in the image, unless it's *very* common to have just one child. Also, this means that if you add more children than the last bucket list can have (256 in the image), The Entitytainer will fail an ASSERT.

### Removal

When you remove an entity, its bucket will of course be available to be used by other entities in the future. The way this works is that each bucket list has an index to the *first free bucket*. When you free a bucket, the bucket space is *repurposed* and the *previous value* of the first free bucket is stored there. Then the first free bucket is re-pointed to your newly freed bucket. I call this an *intrinsic linked bucketed slot allocator*. Do I really? No. Maybe. Is there a name for this?

It's kinda neat because it's super fast to "allocate" or deallocate a bucket, and yet it needs practically no memory for bookkeeping.


## License

Dual licensed under Public Domain and MIT.
