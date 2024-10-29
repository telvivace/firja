# firja
This is a 2D physics engine in early-stage development.
Current features:
- single-threaded collision detection
- single-threaded velocity vector and position updates
- single-threaded graphics engine written in Vulkan (->[repository](https://repo.ijs.si/kema/orbit-rts-graphics))


The intention is to:
- Have a very large 2D space with gravitational attractors
- Have a very large number of objects (200.000)
- Move all physics simulation into the GPU, which might prove very hard but very interesting

Later on:
- Multiple clients connecting to a server that are able to simulate the system
- A server that is able to keep things synchronised even with such a large amount of information
- Eventually have relative coordinate systems in a client-heavy mode of simulation
