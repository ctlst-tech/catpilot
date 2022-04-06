# CTLST FMUV5

# TODO

## POSIX impl with c-atom-lib
- [x] Add errno codes. Take them from standard gnu errno.h
- [x] How to integrate osa to c-atom-lib and eswb?
- [x] PTHREAD_MUTEX_INITIALIZER for mutex?
- [x] Replace system includes with local
- [x] Comment eswb tests
- [x] Move c-atom example from mock to main, fix dependencies
- [x] Stubs for getpid and gettimeofday
- [x] Rename board -> bsp, lib -> misc
- [x] Add posix impl as sys lib
- [ ] Add FS wrappers in OSA
    - [x] Add stdio without open, close...
    - [x] Bind freertos posix impl and fatfs wrappers
        - [x] types.h
        - [ ] comment missing implementations of std functions
        - [ ] realize std functions
    - [ ]
- [ ] Add CMakeLists in board directories
- [ ] Add modules as f_specs functions
