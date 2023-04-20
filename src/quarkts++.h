/*!
 * @file quarkts++.h
 * @author J. Camilo Gomez C.
 * @version 1.0.1
 * @note This file is part of the QuarkTS++ distribution.
 * @brief Global inclusion header
 **/


/*
QuarkTS++ - An open-source OS for small embedded applications.
MIT License
C99 and MISRAC 2012 Compliant

Copyright (C) 2012 Eng. Juan Camilo Gomez Cadavid MSc. All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

VISIT https://github.com/kmilo17pet/QuarkTS-cpp TO ENSURE YOU ARE USING THE LATEST
VERSION.


This file is part of the QuarkTS++ OS distribution.
*/

#ifndef QOS_CPP_H
#define QOS_CPP_H

#define QUARKTS_CPP_VERSION         "1.0.1"
#define QUARKTS_CPP_VERNUM          ( 101u )
#define QUARKTS_CPP_CAPTION         "QuarkTS++ OS " QUARKTS_CPP_VERSION

#include "config/config.h"
#include "include/types.hpp"
#include "include/critical.hpp"
#include "include/clock.hpp"
#include "include/timer.hpp"
#include "include/queue.hpp"
#include "include/fsm.hpp"
#include "include/cli.hpp"
#include "include/kernel.hpp"
#include "include/task.hpp"
#include "include/helper.hpp"
#include "include/coroutine.hpp"
#include "include/memory.hpp"
#include "include/util.hpp"
#include "include/bytebuffer.hpp"
#include "include/edgecheck.hpp"
#include "include/response.hpp"
#include "include/trace.hpp"

#endif /*QOS_CPP_H*/


/**
*  @defgroup qos Operative System
*  This section contains the documentation related to all the Interfaces and 
*  intrinsic definitions of the operating system.
*/

/**
*  @defgroup qtaskcreation Scheduler Interface
*  @ingroup qos
*/

/**
*  @defgroup qtaskmanip Managing tasks
*  @ingroup qos
*/

/**
*  @defgroup qpriv Inter-Task communication
*  @brief Inter-Task communication primitives : Queues, Notifications and Event-flags
*  @ingroup qos
*/

/**
*  @defgroup qqueues Queues
*  @ingroup qpriv
*/

/**
*  @defgroup qnot Notifications
*  @ingroup qpriv
*/

/**
*  @defgroup qeventflags Event Flags
*  @ingroup qpriv
*/

/**
*  @defgroup qclock Clock
*  @ingroup qos
*/

/**
*  @defgroup qcritical Critical
*  @ingroup qos
*/

/**
*  @defgroup qtypes Types and macros
*  @ingroup qos
*/

/**
*  @defgroup qmodules Kernel extensions
*  This section contains the documentation related to all the extensions that 
*  adds additional functionality to the operating system.
*/

/**
*  @defgroup qfsm Finite State Machines
*  @ingroup qmodules
*/

/**
*  @defgroup qstimers Software-Timers
*  @ingroup qmodules
*/

/**
*  @defgroup qcoroutines Co-Routines
*  @ingroup qmodules
*/

/**
*  @defgroup qatcli AT Command Line Interface
*  @ingroup qmodules
*/

/**
*  @defgroup qmemmang Memory Management
*  @ingroup qmodules
*/

/**
*  @defgroup qutility Utilities
*  A collection of utility libraries for the QuarkTS OS
*/

/**
*  @defgroup qlists Generic double-linked lists
*  @ingroup qutility
*/

/**
*  @defgroup qbsbuffers Byte-Sized buffers
*  @ingroup qutility
*/

/**
*  @defgroup qedgecheck I/O Groups for edge checking
*  @ingroup qutility
*/

/**
*  @defgroup qioutils I/O Utils
*  @ingroup qutility
*/

/**
*  @defgroup qresponse Response handler
*  @ingroup qutility
*/

/**
*  @defgroup qtrace Trace
*  @ingroup qutility
*/

/**
*  @defgroup qflm General purpose macros
*  @ingroup qutility
*/
