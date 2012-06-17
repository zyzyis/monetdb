/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is the MonetDB Database System.
 * 
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2012 MonetDB B.V.
 * All Rights Reserved.
*/
#include "monetdb_config.h"
#include "opt_singleton.h"
#include "opt_aliases.h"
#include "mal_interpreter.h"	/* for showErrors() */
#include "mal_builder.h"

/*
 * When we run into an instruction that can not be optimized,
 * we have to rebuild the singleton set as an ordinary BAT.
 */
typedef struct {
	int vidx;
	int hidx;
	int htpe;
	int tidx;
	int ttpe;
	int packed;
} STtuple;

static int
STalias(int idx, int top, STtuple * tuples)
{
	int i;

	for (i = 0; i < top; i++)
		if (tuples[i].vidx == idx)
			return i;
	return -1;
}

static int
STnewTuple(MalBlkPtr mb, int vidx, int *top, STtuple * tuples)
{
	tuples[*top].packed = 1;
	tuples[*top].vidx = vidx;
	tuples[*top].htpe = getHeadType(getVarType(mb, vidx));
	tuples[*top].htpe= TYPE_oid;
	tuples[*top].hidx = newTmpVariable(mb, tuples[*top].htpe);
	tuples[*top].ttpe = getTailType(getVarType(mb, vidx));
	tuples[*top].tidx = newTmpVariable(mb, tuples[*top].ttpe);
	*top = *top + 1;
	return *top - 1;
}

static void
STunpack(MalBlkPtr mb, int varid, STtuple * tuples)
{
	InstrPtr p = 0;

	if (tuples[varid].packed == 0)
		return;

	p = newFcnCall(mb, batRef, unpackRef);
	getArg(p, 0) = tuples[varid].hidx;
	setVarUsed(mb,getArg(p,0));
	setArgument(mb, p, 1, tuples[varid].tidx);
	setVarUsed(mb,getArg(p,1));
	p->retc++;
	p= pushArgument(mb, p, tuples[varid].vidx);
	tuples[varid].packed = 0;
}

static void
STpack(MalBlkPtr mb, int varid, STtuple * tuples)
{
	InstrPtr p = 0;

	p = newFcnCall(mb, batRef, packRef);
	getArg(p, 0) = tuples[varid].vidx;
	p->retc = 1;
	p->argc = 1;
	p= pushArgument(mb, p, tuples[varid].hidx);
	p= pushArgument(mb, p, tuples[varid].tidx);
}

static int
STevaluate(Client cntxt, MalBlkPtr mb, int cnt, int *vars)
{
	int i, j = 0, k, limit, slimit, actions = 0;
	InstrPtr p;
	InstrPtr *old;
	STtuple *tuples;
	int top = 0;

	(void) cntxt;
	if( mb->errors) 
		return 0;
	tuples = (STtuple*) GDKmalloc(mb->stop * sizeof(STtuple));
	if (tuples == NULL)
		return 0;
	limit = mb->stop;
	slimit = mb->ssize;
	old = mb->stmt;

	/*
	 * Prepare the list of tuple aliases using allocation of
	 * variables on the stack.
	 */
	for (i = 0; i < cnt; i++)
		STnewTuple(mb, vars[i], &top, tuples);
	OPTDEBUGsingleton{
		mnstr_printf(cntxt->fdout, "START SINGLETON OPTIMIZER %d:", cnt);
		for (i = 0; i < top; i++)
			mnstr_printf(cntxt->fdout, "[%d,%d,%d] ", vars[i], tuples[i].hidx, tuples[i].tidx);
		mnstr_printf(cntxt->fdout, "\n");
	}
	if ( newMalBlkStmt(mb, mb->ssize) < 0){
		GDKfree(tuples);
		return 0;
	}

	for (i = 0; i < limit; i++) {
		p = old[i];
		/* decode operations */
		if (getModuleId(p) == batRef) {
			if (getFunctionId(p) == reverseRef && 
				(k = STalias(getArg(p, 1), top, tuples) >= 0)) {
				setModuleId(p, NULL);
				setFunctionId(p, NULL);
				p->retc = p->argc = 2;
				p= pushArgument(mb, p, tuples[k].hidx);
				p= pushArgument(mb, p, tuples[k].tidx);
				k = STnewTuple(mb, getArg(p, 0), &top, tuples);
				p->argv[0] = tuples[k].hidx;
				p->argv[1] = tuples[k].tidx;
				p->token= ASSIGNsymbol;
				actions++;
			} else
				goto STrepack;
		} else if (getModuleId(p)== ioRef) {
			if (getFunctionId(p) == printRef &&
				p->argc == 3 && 
				(k = STalias(getArg(p, 1), top, tuples)) >= 0) {
				if (tuples[k].packed == 0) {
					getArg(p, 1) = tuples[k].hidx;
					p= pushArgument(mb, p, tuples[k].tidx);
				} else
					goto STrepack;
			} else
				goto STrepack;
		} else if (getModuleId(p) == algebraRef) {
			if (putName("markH", 5) == getFunctionId(p) && 
				(k = STalias(getArg(p, 1), top, tuples)) >= 0) {
				STunpack(mb, k, tuples);
				setModuleId(p, NULL);
				setFunctionId(p, NULL);
				p->retc = p->argc = 2;
				p= pushOid(mb, p,0);
				p= pushArgument(mb, p, tuples[k].tidx);
				k = STnewTuple(mb, getArg(p, 0), &top, tuples);
				tuples[k].packed = 0;
				p->argv[0] = tuples[k].hidx;
				setVarUsed(mb,getArg(p,0));
				p->argv[1] = tuples[k].tidx;
				setVarUsed(mb,getArg(p,1));
				p->token = ASSIGNsymbol;
				p->fcn = NULL;
				actions++;
			} else if (getFunctionId(p) == markTRef && 
						(k = STalias(getArg(p, 1), top, tuples)) >= 0) {
				STunpack(mb, k, tuples);
				setModuleId(p, NULL);
				setFunctionId(p, NULL);
				p->retc = p->argc = 2;
				p= pushArgument(mb, p, tuples[k].hidx);
				p= pushOid(mb, p,0);
				k = STnewTuple(mb, getArg(p, 0), &top, tuples);
				tuples[k].packed = 0;
				p->argv[0] = tuples[k].hidx;
				setVarUsed(mb,getArg(p,0));
				p->argv[1] = tuples[k].tidx;
				setVarUsed(mb,getArg(p,1));
				p->token = ASSIGNsymbol;
				p->fcn = NULL;
				actions++;
			} else if (getFunctionId(p) == joinRef) {
				int k1 = STalias(getArg(p, 1), top, tuples);
				int k2 = STalias(getArg(p, 2), top, tuples);

				if (k1 >= 0 && k2 >= 0) {
					if (tuples[k2].htpe == TYPE_oid && tuples[k1].htpe == TYPE_oid) {
						STunpack(mb, k1, tuples);
						STunpack(mb, k2, tuples);
						printf("MERGE VOID JOIN \n");
						setModuleId(p, NULL);
						setFunctionId(p, NULL);
						STnewTuple(mb, getArg(p, 0), &top, tuples);
						p->argc = 0;
						p= pushReturn(mb, p, tuples[top - 1].hidx);
						p= pushReturn(mb, p, tuples[top - 1].tidx);
						p->retc = 2;
						p= pushArgument(mb, p, tuples[k1].hidx);
						p= pushArgument(mb, p, tuples[k2].tidx);
						p->token = ASSIGNsymbol;
						p->fcn = NULL;
					}
				} else if (k1 >= 0) {
					goto STrepack;
				} else if (k2 >= 0) {
					goto STrepack;
				} else
					goto STrepack;
				/* just push instruction */
			} else
/*
				putName("kunion",6)== getFunctionId(p) ||
				getFunctionId(p) == joinRef ||
				putName("fetchjoin",9)== getFunctionId(p) ||
				putName("mergejoin",9)== getFunctionId(p) ||
				putName("indexjoin",9)== getFunctionId(p) ||
				putName("hashjoin",8)== getFunctionId(p) ||
				putName("cartesianproduct",16)== getFunctionId(p) ||
				putName("union",5)== getFunctionId(p) )
*/
			if (getFunctionId(p) == countRef && 
				(k = STalias(getArg(p, 1), top, tuples)) >= 0) {
				STunpack(mb, k, tuples);
				/* count propagation */
				setModuleId(p, NULL);
				setFunctionId(p, NULL);
				p->argc = 1;
				p->token = ASSIGNsymbol;
				p= pushLng(mb,p,1);
			} else {
			STrepack:
				/* pack everything you need */
				for (j = 0; j < p->argc; j++)
					if ((k = STalias(getArg(p, j), top, tuples)) >= 0 && tuples[k].packed == 0)
						STpack(mb, k, tuples);
			}
		} else if (putName("datacell", 8) == getModuleId(p)) {
			if (putName("bindReceptor", 12) && (k = STalias(getArg(p, 0), top, tuples)) >= 0) {
				setFunctionId(p, putName("bindReceptorTuple", 17));
				getArg(p, 0) = tuples[k].hidx;
				setArgument(mb, p, 1, tuples[k].tidx);
				p->retc++;
				tuples[k].packed = 0;
			} else if (putName("keep", 4) && p->argc >= 3 && (k = STalias(getArg(p, 3), top, tuples)) >= 0) {
				getArg(p, 3) = tuples[k].tidx;
			}
		} else {
			/* check for any target alias */
			for (j = 0; j < p->retc; j++)
				for (k = 0; k < top; k++)
					if (getArg(p, j) == tuples[k].vidx) {
						getArg(p, j) = tuples[k].hidx;
						setArgument(mb, p, j + 1, tuples[k].tidx);
						p->retc++;
					}
		}
		pushInstruction(mb, p);
	}
	for(; i<slimit; i++)
	if( old[i])
		freeInstruction(old[i]);
	GDKfree(old);
	OPTDEBUGsingleton
	if (actions) {
		printf("FINAL STAGE actions=%d\n", actions);
		printFunction(cntxt->fdout, mb, 0, LIST_MAL_ALL);
		for (i = 0; i < top; i++)
			mnstr_printf(cntxt->fdout, "[%d,%d:%s,%d:%s] ", tuples[i].vidx, tuples[i].hidx, getTypeName(tuples[i].htpe), tuples[i].tidx, getTypeName(tuples[i].ttpe));
		mnstr_printf(cntxt->fdout, "\n");
	}

	OPTDEBUGsingleton{
		printf("FINAL STAGE SINGLETON errors=%d\n", mb->errors);
		printFunction(cntxt->fdout, mb, 0, LIST_MAL_ALL);
	}
	GDKfree(tuples);
	return actions;
}

int
OPTsingletonImplementation(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p)
{
	int *vars;
	int cnt = 0, i, j;

	(void) cntxt;
	(void) stk;
	vars = (int *) GDKmalloc(mb->vtop);
	if (vars == NULL)
		return 0;

	for (i = 1; i < mb->stop; i++) {
		p = getInstrPtr(mb, i);
		for (j = 0; j < p->retc; j++)
			if (varGetProp(mb, getArg(p, j), singletonProp) != NULL)
				vars[cnt++] = getArg(p, j);
	}
	i= STevaluate(cntxt, mb, cnt, vars);
	GDKfree(vars);
	return i;
}
