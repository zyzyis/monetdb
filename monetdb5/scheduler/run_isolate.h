/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _RUN_ISOLATION
#define _RUN_ISOLATION
#include "mal.h"
#include "mal_instruction.h"
#include "mal_client.h"

#define DEBUG_RUN_ISOLATION

#ifdef WIN32
#if !defined(LIBMAL) && !defined(LIBATOMS) && !defined(LIBKERNEL) && !defined(LIBMAL) && !defined(LIBOPTIMIZER) && !defined(LIBSCHEDULER) && !defined(LIBMONETDB5)
#define run_isolate_export extern __declspec(dllimport)
#else
#define run_isolate_export extern __declspec(dllexport)
#endif
#else
#define run_isolate_export extern
#endif

run_isolate_export str RUNisolation(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p);
#endif /* MAL_RUN_ISOLATION */

