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
        - [x] comment missing implementations of std functions
        - [x] realize std functions
- [x] Start c-atom on fmuv5
    - [x] With empty function
    - [x] Add IMU in spec update function
- [x] Add stdout, stdin, stderr streams in cli
    - [x] Add stdout stub for printf use
    - [ ] Problems with vasprintf (memory? stack overflow?)
        - [x] Add wrap for malloc, calloc, free (from heap_3)
- [ ] Add CMakeLists in board directories
- [ ] Add modules as f_specs functions
