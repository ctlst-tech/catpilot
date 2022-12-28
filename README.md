# CatPilot

Catpilot is an open-source library for drone autopilots. It is an essential of the project 
[uas-catpilot](https://github.com/ctlst-tech/uas-catpilot). 

The core of this library is a platform-independent embedded framework - 
[c-atom](https://github.com/ctlst-tech/c-atom). CatPilot also provides board support package and operating system 
integration of the C-ATOM.

This library should be used as a submodule to the main project of the drone control system.

# Project structure
- bsp - drivers and high abstraction layer to work with a specific autopilot
- [c-atom](https://github.com/ctlst-tech/c-atom) - framework for embedded systems
- atomics - board specific atomic functions
- os - operating systems and POSIX API wrappers implementations

For more info refer to the [documentation](https://docs.ctlst.app/uas-catpilot/intro.html)
