/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _MAL_REDUCE_
#define _MAL_REDUCE_
#include "opt_prelude.h"
#include "opt_support.h"
#include "mal_instruction.h"

#define OPTDEBUGreduce  if ( optDebug & ((lng)1 <<DEBUG_OPT_REDUCE) )

opt_export int OPTreduceImplementation(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p);

#endif
