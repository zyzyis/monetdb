/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _OPT_CONSTANTS_
#define _OPT_CONSTANTS_
#include "opt_prelude.h"
#include "opt_support.h"
#include "mal_exception.h"

opt_export int OPTconstantsImplementation(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p);

#define OPTDEBUGconstants  if ( optDebug & ((lng) 1 <<DEBUG_OPT_CONSTANTS) )

#endif
