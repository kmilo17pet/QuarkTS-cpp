[![Built for](https://img.shields.io/badge/built%20for-microcontrollers-lightgrey?logo=WhiteSource)](https://github.com/kmilo17pet/QuarkTS-cpp)
[![CodeFactor](https://www.codefactor.io/repository/github/kmilo17pet/quarkts-cpp/badge)](https://www.codefactor.io/repository/github/kmilo17pet/quarkts-cpp)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b3d75b3d6e2c4733ab87fd8bab6f4dea)](https://app.codacy.com/gh/kmilo17pet/QuarkTS-cpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![DeepSource](https://deepsource.io/gh/kmilo17pet/QuarkTS-cpp.svg/?label=active+issues&show_trend=true&token=MlFRbA4h9mtWn6ZQC_UD8aYZ)](https://deepsource.io/gh/kmilo17pet/QuarkTS-cpp/?ref=repository-badge)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/kmilo17pet/QuarkTS-cpp?logo=webpack)](https://github.com/kmilo17pet/QuarkTS-cpp/releases)
[![MISRAC++2008](https://img.shields.io/badge/MISRAC++2008-Compliant-blue.svg?logo=verizon)](https://www.misra.org.uk/)
[![CERT](https://img.shields.io/badge/CERT-Compliant-blue.svg?logo=cplusplus)](https://wiki.sei.cmu.edu/confluence/display/seccode/SEI+CERT+Coding+Standards)
[![C++ Standard](https://img.shields.io/badge/STD-C++11-green.svg?logo=cplusplus)](https://en.cppreference.com/w/cpp/11)
[![Build](https://github.com/kmilo17pet/QuarkTS-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/kmilo17pet/QuarkTS-cpp/actions/workflows/build.yml)
[![Documentation](https://github.com/kmilo17pet/QuarkTS-cpp/actions/workflows/doxygen_gen.yml/badge.svg)](https://github.com/kmilo17pet/QuarkTS-cpp/actions/workflows/doxygen_gen.yml)
[![arduino-library-badge](https://www.ardu-badge.com/badge/QuarkTS.svg?)](https://www.ardu-badge.com/QuarkTS)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg?logo=textpattern)](https://github.com/kmilo17pet/QuarkTS-cpp/graphs/commit-activity)
[![License](https://img.shields.io/github/license/kmilo17pet/QuarkTS-cpp?logo=livejournal)](https://github.com/kmilo17pet/QuarkTS-cpp/blob/master/LICENSE)
[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fkmilo17pet%2FQuarkTS-cpp&count_bg=%2379C83D&title_bg=%23555555&icon=&icon_color=%23E7E7E7&title=hits&edge_flat=false)](https://github.com/kmilo17pet/QuarkTS-cpp)

![Logo](https://github.com/kmilo17pet/QuarkTS-cpp/raw/main/doc/quarktslogo.png)

# QuarkTS++ : An open-source OS for small embedded applications.

QuarkTS++is the [QuarkTS](https://github.com/kmilo17pet/QuarkTS) port for C++.

* Download the latest release [here](https://github.com/kmilo17pet/QuarkTS-cpp/releases)
* Read the documentation online [here](https://kmilo17pet.github.io/QuarkTS-cpp/)

QuarkTS++ is an operating system that provides a modern environment to build stable and predictable event-driven multitasking embedded software. The OS is built on top of a cooperative quasi-static scheduler and its simplified kernel implements a specialized round-robin scheme using a linked-chain approach and an event-queue to provide true FIFO priority-scheduling.

## Features:

-   Prioritized cooperative scheduling.
-   Time control (Timed tasks and software timers)
-   Inter-Task communication primitives, queues, notifications and event-flags.
-   State-Machines ( hierarchical support )
-   Stackless Co-routines.
-   AT Command Line Interface (CLI)

QuarkTS++ is developed using a formal and rigorous process framed in compliance with the MISRA C++ 2008 and CERT coding standard guidelines and complemented with multiple static-analysis checks targeted to safe critical applications.
QuarkTS++ builds with many different compilers, some of which are more advanced than others. For that reason, QuarkTS++ does not use any of the features or syntax that have been introduced to the C++ language by, or since, the C++11 standard.

### Why cooperative?

Rather than having preemption, tasks manage their own life-cycle. This brings significant benefits, fewer re-entrance problems are encountered, because tasks cannot be interrupted arbitrarily by other tasks, but only at positions permitted by the programmer, so you mostly do not need to worry about pitfalls of the concurrent approach (resource-sharing, race-conditions, deadlocks, etc...). 

### What is it made for?

The design goal of QuarkTS is to achieve its stated functionality using a small, simple, and (most importantly) robust implementation to make it suitable on resource-constrained microcontrollers, where a full-preemptive RTOS is an overkill and their inclusion adds unnecessary complexity to the firmware development. In addition with a state-machines support, co-routines, time control and the inter-task communication primitives, QuarkTS provides a modern environment to build stable and predictable event-driven multitasking embedded software. Their modularity and reliability make this OS a great choice to develop efficiently a wide range of applications in low-cost devices, including automotive controls, monitoring and Internet of Things.

### Why should I choose it?

QuarkTS++ is not intended to replace o compete with the other great and proven RTOS options already available today, for example [FreeRTOS](https://freertos.org/) or [MicroC/OS-II](https://www.micrium.com/rtos/), in fact, you should check these options first. However, due to its size and features, is intended to play in the space between RTOSes and bare-metal. QuarkTS was written for embedded developers who want more functionality than what existing task schedulers offer, but want to avoid the space and complexity of a full RTOS, keeping the taste of a robust and safe one. 

