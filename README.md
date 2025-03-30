#  GPU By Example

This repository goes along with the "Muddling through SDL GPU" set of posts
at https://www.jonathanfischer.net/. It's basically me, someone who never
moved beyond OpenGL 1.2, learning how to use a modern GPU API, and shaders,
and all that fun stuff.

## Building and Running examples

I've tried to make this as easy as I can: the necessary libraries are included in this
repository, and everything should be linked up and ready to run in the normal dev tools
for each platform.

### macOS

You'll probably need to remove the quarantine xattr from the repository before Xcode will
work without complaining at you. To do that, pop open terminal and run

    xattr -r -d com.apple.quarantine /path/to/repository

Open `GpuByExample.xcworkspace` in Xcode. Each example is included as a separate project:
you can run them by selecting the example you want to run in the Build Scheme dropdown
and running it. I've left code signing turned off, SDL's included as an xcframework, and
the necessary resources get copied into each example's app bundle, so it should just
work.

![Xcode](/Images/xcode.png "Build Scheme dropdown in Xcode 16.1")

### Windows

Open `GpuByExample.sln` in Visual Studio. Each example is included as a separate project:
you can run then by right clicking the example in the Solution Explorer and selecting
"Set as Startup Project", then run or debug it. The necessary SDL libraries for x86 and
x86_64 are included in the Libraries directory in the repository, and each example should
be configured to both link the appropriate .lib files and find the appropriate .dll files
at runtime. Each example should also be configured to run with its working directory as
the appropriate resources directory, so it'll find any necessary shaders or other resources
at runtime.

> Quick caveat: I can't verify that this works at the moment. The computer I
> use to build the examples is 13 years old and has integrated graphics; it won't run any
> of this. I borrow a little time to test these out on my son's gaming PC, but I don't
> have Visual Studio installed up there.

![Visual Studio](/Images/visual-studio.png "Set as Startup Project in Visual Studio 2022")

### Linux

This is more manual, sorry: I haven't taken time to make this automatic yet. First, SDL
is included as a submodule of this repository, so you'll need to make sure you're pulling
that down as well. Either clone it with `git clone --recursive` in the first place, or
after cloning it run `git submodule init` followed by `git submodule update` to clone
SDL.

To build and run an example, open up your terminal and switch to the example's
directory. Run:

    cmake -B build/

to generate the necessary build files for the example, then

    cmake --build build/

to actually build the example. Each example will build its own copy of SDL, sorry, I'm not
sure how to avoid that yet. Copy the contents of the example's Resources directory into
the build/ directory, then switch into that directory and run the example: it'll be named
`gbe-example<something>`. You'll need to make sure it can find the SDL library with 
`LD_LIBRARY_PATH`. So the full set of steps for Example 2 would look like:

    cd /path/to/repo/Example2-DrawingPrimitives
    cmake -B build/
    cmake --build build/
    cd build/
    cp -r ../Resources/* ./
    LD_LIBRARY_PATH=`pwd` ./gbe-example2-drawing-primitives

