Thanatos
========
Or Theta.  I haven't decided on a final name yet.

So what is this?
----------------
This port is a work in progress.  However, the goal is to move Doom's gameloop into Javascript for the purposes of making it easy to tinker with and mod.  In fact, I'd like to get to a point where this port is more like a game engine that just so happens to play a reasonable facsimile of Doom out of the box.

There are also a few other "nice to have" features that I'd also like to implement - most notably support for arbitrary resolutions, an uncapped framerate, and an ingame console.  If you're curious for more information, check out the [Development Roadmap](https://github.com/AlexMax/thanatos/wiki/Development-Roadmap).

So what isn't this?
-------------------
This port is _not_ designed to run in a web browser, and there is no relation to Node.js.  However, if you are familiar with Javascript of any stripe, you will feel right at home.

Aside from that, this port is also primarily designed to be tinkered with, first and foremost.  Making it the most tinker-able port has led sacrifices in other areas.

- It is not the fastest port.  If you want to benchmark [nuts.wad](https://www.youtube.com/watch?v=mt8kGiFBe_I) or gawk at [The Hag's Finger](https://www.youtube.com/watch?v=24POUqOWkg4), I would recommend a more performant port.
- It is not the most accurate port.  At this point, the port aims for "decent" demo compatibility, purely in service of making sure the translated code works the same as the original C.  However, the original Doom had many bugs, and demo compatibility necessitates emulating nearly all of them.  So at some point in the future, demo support might get tossed.
- It is not the port that can play the majority of Doom modifications flawlessly.  Doom predates Quake 2's modular design, and most Doom mods are loaded via `.WAD` files that rely on the engine to parse dozens of domain-specific data lumps, which considerably complicates the codebase.  Since [GZDoom](coelckers/gzdoom) already exists, I've decided not to prioritize supporting those kinds of mods.

Compiling
---------
This software requires a reasonably up-to-date C++ compiler that supports C++14.  I'm using Visual Studio 2015 as my primary development environment, from what I've read GCC 5 and Clang 3.4 should suffice.

You need [CMake](https://cmake.org/) to build the software.

You also need [SDL2](https://www.libsdl.org/download-2.0.php), [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/) and [SDL2_net](https://www.libsdl.org/projects/SDL_net/).  If you're on Linux, you can install the development packages for them, as they're likely included in your repository.  Windows users can simply extract the development libraries into any directory, and then point CMake at them when you configure the build.

License
-------
GPLv2.  Unfortunately, I cannot make any accomodations on the license.  Doom was originally open-sourced under both an educational, non-commercial license and the GNU GPLv2, and Chocolate Doom (the original software I forked from) is exclusively GPLv2.
