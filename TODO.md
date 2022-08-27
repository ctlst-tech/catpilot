# CTLST FMUV5 TODO
## POSIX impl with c-atom
- [x] Add errno codes. Take them from standard gnu errno.h
- [x] How to integrate osa to c-atom-lib and eswb?
- [x] PTHREAD_MUTEX_INITIALIZER for mutex?
- [x] Replace system includes with local
- [x] Comment eswb tests
- [x] Move c-atom example from mock to main, fix dependencies
- [x] Stubs for getpid and gettimeofday
- [x] Rename board -> bsp, lib -> misc
- [x] Add posix impl as sys lib
- [x] Add FS wrappers in OSA
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
  - [x] Problems with vasprintf (memory? stack overflow?)
      - [x] Add wrap for malloc, calloc, free (from heap_3)
- [x] pthread_setname_np/pthread_getname_np implementation
- [ ] pthread_cancel wrapper
  - This function doesn't work in existing implementations
- [ ] stubs for tcp stack
- [x] <termios.h> wrappers
- [x] Add high wrapper layer for open/write/read/close
- [ ] Add CMakeLists in board directories
- [ ] Add modules as f_specs functions

## Hardware
- [x] Add ICM20689 driver
- [x] Add BMI055 driver
- [ ] Add MS5611 driver
- [ ] Tests with statistics
  - [ ] Max freq
  - [ ] Collizions on the SPI1 bus
  - [ ] MCU load

## Utils
- [x] Add log func with types of msg and module

## Porting to Cube Orange
- [x] Add core sources
- [x] Rewrite RCC
- [ ] Rewrite DMA
- [ ] Rewrite EXTI
- [ ] Add ICM20948 driver
- [ ] Add ICM20649 driver
- [x] Update pinout

