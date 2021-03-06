/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _SQL_EXECUTE_H_
#define _SQL_EXECUTE_H_
#include "sql.h"

sql5_export str SQLstatementREST(Client c, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
sql5_export str SQLstatementIntern(Client c, str *expr, str nme, int execute, bit output, res_table **result);
sql5_export str SQLexecutePrepared(Client c, backend *be, cq *q);
sql5_export str SQLengineIntern(Client c, backend *be);
sql5_export str SQLrecompile(Client c, backend *be);

#endif /* _SQL_EXECUTE_H_ */
