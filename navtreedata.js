/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "OS", "index.html", [
    [ "User manual", "index.html", null ],
    [ "Overview", "q_background.html", [
      [ "About the OS", "q_background.html#q_about", null ],
      [ "License", "q_background.html#q_license", null ],
      [ "Hardware compatibility", "q_background.html#q_hardwarecompatibility", null ],
      [ "Development process and coding standard", "q_background.html#q_codstandard", null ],
      [ "Memory usage", "q_background.html#q_memusage", null ]
    ] ],
    [ "Fundamentals", "q_foundations.html", [
      [ "RTC(Run To Completion) model", "q_foundations.html#rtcmodel", null ],
      [ "Timing Approach", "q_foundations.html#q_timmingapproach", null ],
      [ "Tasks", "q_foundations.html#q_tasks", [
        [ "The idle task", "q_foundations.html#q_idletask", null ],
        [ "Adding tasks to the scheme", "q_foundations.html#q_addtasks", null ],
        [ "Event-triggered tasks", "q_foundations.html#q_eventtasks", null ],
        [ "Removing a task", "q_foundations.html#q_removetask", null ]
      ] ],
      [ "Running the OS", "q_foundations.html#q_runos", [
        [ "Releasing the scheduler", "q_foundations.html#q_releasescheduler", null ]
      ] ],
      [ "Global states and scheduling rules", "q_foundations.html#q_globalstates", [
        [ "Rules", "q_foundations.html#q_osrules", [
          [ "Event precedence", "q_foundations.html#q_os_evenprecedence", null ],
          [ "Additional operational states", "q_foundations.html#q_os_operational_states", null ]
        ] ]
      ] ],
      [ "Critical sections", "q_foundations.html#q_critical", [
        [ "Examples", "q_foundations.html#q_critical_examples", [
          [ "Registering the interrupt control functions", "q_foundations.html#regintfcns", null ],
          [ "Using critical::scope for quick inline protection", "q_foundations.html#usecritscope", null ],
          [ "Using critical::lock (RAII-style)", "q_foundations.html#usecritlock", null ]
        ] ]
      ] ],
      [ "Configuration macros", "q_foundations.html#q_configmacros", null ]
    ] ],
    [ "Getting started", "q_gettingstarted.html", [
      [ "Getting the OS", "q_gettingstarted.html#q_getos", [
        [ "Cloning QuarkTS++", "q_gettingstarted.html#q_cloneos", null ],
        [ "Including QuarkTS++ as a git sub-module", "q_gettingstarted.html#q_submodule", null ],
        [ "Get a copy of the OS configuration file", "q_gettingstarted.html#q_getqconfig", null ]
      ] ],
      [ "First steps", "q_gettingstarted.html#q_firststeps", null ],
      [ "Two simple demonstrative examples", "q_gettingstarted.html#q_os_demo", [
        [ "A simple scheduling", "q_gettingstarted.html#q_os_example1", null ],
        [ "Using the task argument", "q_gettingstarted.html#q_os_example2", null ]
      ] ],
      [ "Using QuarkTS++ in Arduino", "q_gettingstarted.html#qarduinousage", null ]
    ] ],
    [ "Events", "q_events.html", [
      [ "Retrieving the event data", "q_events.html#eventdata", null ],
      [ "The Time-Elapsed event", "q_events.html#q_event_timeelapsed", null ],
      [ "Asynchronous events and inter-task communication", "q_events.html#q_asyncevents", [
        [ "Notifications", "q_events.html#q_notifications", [
          [ "Simple Notifications", "q_events.html#q_simplenot", null ],
          [ "Queued Notifications", "q_events.html#q_queuednot", null ],
          [ "Sending notifications", "q_events.html#q_notificationsend", null ],
          [ "Spread a notification", "q_events.html#qnotspread", null ]
        ] ],
        [ "Queues", "q_events.html#q_queues", [
          [ "Setting up a queue", "q_events.html#q_queuesetup", null ],
          [ "Attach a queue to a task", "q_events.html#q_attachqueue", null ],
          [ "A queue example", "q_events.html#q_queueexample", null ]
        ] ],
        [ "Event Flags", "q_events.html#qeventflags", [
          [ "Using the task Event-flags", "q_events.html#q_eventflagsexample", null ]
        ] ]
      ] ]
    ] ],
    [ "Extensions", "q_extensions.html", "q_extensions" ],
    [ "Topics", "topics.html", "topics" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"classq_o_s_1_1sm_1_1state.html#a0fae78b92703668ad67b43f03ec72226",
"group__qflm.html#gaf3383fec42786ce57d66b957201b59d4",
"q_events.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';