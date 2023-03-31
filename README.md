[![CodeFactor](https://www.codefactor.io/repository/github/kmilo17pet/quarkts-cpp/badge)](https://www.codefactor.io/repository/github/kmilo17pet/quarkts-cpp)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b3d75b3d6e2c4733ab87fd8bab6f4dea)](https://app.codacy.com/gh/kmilo17pet/QuarkTS-cpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![DeepSource](https://deepsource.io/gh/kmilo17pet/QuarkTS-cpp.svg/?label=active+issues&show_trend=true&token=MlFRbA4h9mtWn6ZQC_UD8aYZ)](https://deepsource.io/gh/kmilo17pet/QuarkTS-cpp/?ref=repository-badge)

# qOS: QuarkTS++

qOS is the [QuarkTS](https://github.com/kmilo17pet/QuarkTS) port for C++.

qOS is an operating system that provides a modern environment to build stable and predictable event-driven multitasking embedded software. The OS is built on top of a cooperative quasi-static scheduler and its simplified kernel implements a specialized round-robin scheme using a linked-chain approach and an event-queue to provide true FIFO priority-scheduling.

## Features:

-   Prioritized cooperative scheduling.
-   Time control (Timed tasks and software timers)
-   Inter-Task communication primitives, queues, notifications and event-flags.
-   State-Machines ( hierarchical support )
-   Stackless Co-routines.
-   AT Command Line Interface (CLI)

qOS is developed using a formal and rigorous process framed in compliance with the MISRA C++ 2012 and CERT coding standard guidelines and complemented with multiple static-analysis checks targeted to safe critical applications.


 :construction: :construction_worker: Currently under development... :construction_worker: :construction:

:warning: currently qOS is in an early stage of development, any contribution is more than welcome :blush:

qOS builds with many different compilers, some of which are more advanced than others. For that reason qOS does not use any of the features or syntax that have been introduced to the C language by, or since, the C++11 standard.

### how to build

  
```sh
cd build
cmake ../src
make
```