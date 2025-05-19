# firja
![vizualizirana simulacija](https://github.com/telvivace/firja/blob/mr25/art/firja_screenshot.png)

This is a 2D physics engine in early-stage development.

It is capable of simulating circular objects of varying sizes in a 2D space. It uses a 2-dimensional k-d
tree, and runs pretty well (20fps) with object counts of up to 200.000 on a single thread. It is also described
in an article for the Annual Meeting of Young Researchers of Slovenia 2025 (->[article](https://github.com/telvivace/firja-doc))

## Purpose:
I want to make a videogame with a very large and complex physically simulated world. It would be
an RTS game with intelligent and self-sufficient units that do not require a lot of micro-management.
It might prove very interesting to try to train unit AI that knows how to do things on it's own, and
in the recent machine learning hype, I might very well find the resources for that, especially if the
physics is GPU-friendly.

I have not yet heard of a videogame that has fully GPU-driven physics, and I want to try to make one.
It's a very large bite for a teenager, but I surely hope that I at least learn something. The thing
I'm afraid of the most is that it would be too challenging to synchronise such large systems over
an internet connection and the logical nightmare that is synchronising GPU memory with a remote server.

## Current features:

- single-threaded collision detection
- single-threaded velocity vector and position updates
- single-threaded graphics engine written in Vulkan (SDL is used in the current version, because it's mostly for debugging) (->[repository](https://repo.ijs.si/kema/orbit-rts-graphics))
- gravity
- ability to simulate 200.000 objects at a usable speed (20fps)

## Goals:
- Have a very large 2D space with gravitational attractors (Kepler orbits)
- Move all physics simulation into the GPU, which might prove very hard but very interesting
- rewrite the engine into Rust (it's progressively harder to debug larger C projects, and my coding
style doesn't prove trustworthy enough for me to invest this much effort into a potentially horrifying abomination
that is bug-ridden and memory-unsafe)
- upgrade some of the algorithms on my k-d tree that might significantly speed up the simulation (3x)
- find a way to balance a tree of lists without slowing down too much (maybe irrelevant if I plan
to put it on the GPU)



### Later on:
- Multiple clients connecting to a server that are able to simulate the system and synchronise the
data through the server
- A server that is able to keep things synchronised even with such a large amount of information.
Emphasis is on the fact that the server does not simulate the whole system, so the game world can actually
be very large and hopefully still funciton
- Eventually have relative coordinate systems in a client-heavy mode of simulation that sync up only when players interact with each other
