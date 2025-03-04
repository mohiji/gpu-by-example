#  GPU By Example

This repository goes along with the "Muddling through SDL GPU" set of posts
at https://www.jonathanfischer.net/. It's basically me, someone who never
moved beyond OpenGL 1.2, learning how to use a modern GPU API, and shaders,
and all that fun stuff.

### Building on macOS

Right now this is only set up to build for macOS, sorry. The code should work
on macOS, Windows, and Linux, but I haven't taken the time to set up projects
for the other platforms yet.

To build on macOS, you will first need to download the latest SDL3 release from
https://github.com/libsdl-org/SDL/releases/. Download the .dmg file, open it up,
and drag the SDL3.xcframework out somewhere. (I use ~/Library/Frameworks).
Importantly, macOS is going to set the `com.apple.quarantine` xattr on the
framework, and Xcode is going to complain at you when you try and build and run
with it. Oddly it still works, but it's probably better to get rid of that.

In Terminal.app, run:

    xattr -r -d com.apple.quarantine /path/to/SDL3.xcframework

Then open up the GpuByExample Xcode project. You should see SDL3.framework in the
Project navigator on the left, but it'll show as a broken reference. Click on the
framework, then in the File Inspector on the right correct the path to the framework
by clicking on the tiny folder icon under the Location dropdown.

Once that's done, you should be able to just run the project.
