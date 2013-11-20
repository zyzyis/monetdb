#include "monetdb_config.h"
#include "sql.h"
#include "sql_result.h"
#include "sql_gencode.h"
#include <sql_storage.h>
#include <sql_scenario.h>
#include <store_sequence.h>
#include <sql_optimizer.h>
#include <sql_datetime.h>
#include <rel_optimizer.h>
#include <rel_distribute.h>
#include <rel_select.h>
#include <rel_exp.h>
#include <rel_dump.h>
#include <rel_bin.h>
#include "clients.h"
#include "mal_instruction.h"

str
batstr_2time_timestamp( int *res, int *bid, int *digits )
{
	BAT *b, *dst;
	BATiter bi;
	BUN p,q;
	char *msg = NULL;

	if( (b = BATdescriptor(*bid)) == NULL ){
		throw(SQL, "batcalc.str_2time_timestamp", "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = BATnew(b->htype, TYPE_timestamp, BATcount(b));
	if( dst == NULL){
		BBPreleaseref(b->batCacheid);
		throw(SQL, "sql.timestamp", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATloop(b,p,q) {
		char *v = (char*)BUNtail(bi,p);
		union {
			lng l;
			timestamp r;
		} u;
		msg = str_2time_timestamp( &u.r, &v, digits );
		if (msg)
			break;
		BUNins(dst, BUNhead(bi,p), &u.r, FALSE);
	}
	BBPkeepref( *res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}




str
battimestamp_2time_timestamp( int *res, int *bid, int *digits )
{
	BAT *b, *dst;
	BATiter bi;
	BUN p,q;
	char *msg = NULL;

	if( (b = BATdescriptor(*bid)) == NULL ){
		throw(SQL, "batcalc.timestamp_2time_timestamp", "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = BATnew(b->htype, TYPE_timestamp, BATcount(b));
	if( dst == NULL){
		BBPreleaseref(b->batCacheid);
		throw(SQL, "sql.timestamp", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATloop(b,p,q) {
		timestamp *v = (timestamp*)BUNtail(bi,p);
		union {
			lng l;
			timestamp r;
		} u;
		msg = timestamp_2time_timestamp( &u.r, v, digits );
		if (msg)
			break;
		BUNins(dst, BUNhead(bi,p), &u.r, FALSE);
	}
	BBPkeepref( *res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}




str
batnil_2time_timestamp( int *res, int *bid, int *digits )
{
	BAT *b, *dst;
	BATiter bi;
	BUN p,q;
	char *msg = NULL;

	if( (b = BATdescriptor(*bid)) == NULL ){
		throw(SQL, "batcalc.nil_2time_timestamp", "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = BATnew(b->htype, TYPE_timestamp, BATcount(b));
	if( dst == NULL){
		BBPreleaseref(b->batCacheid);
		throw(SQL, "sql.timestamp", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATloop(b,p,q) {
		void *v = (void*)BUNtail(bi,p);
		union {
			lng l;
			timestamp r;
		} u;
		msg = nil_2time_timestamp( &u.r, v, digits );
		if (msg)
			break;
		BUNins(dst, BUNhead(bi,p), &u.r, FALSE);
	}
	BBPkeepref( *res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}




str
batstr_2time_daytime( int *res, int *bid, int *digits )
{
	BAT *b, *dst;
	BATiter bi;
	BUN p,q;
	char *msg = NULL;

	if( (b = BATdescriptor(*bid)) == NULL ){
		throw(SQL, "batcalc.str_2time_daytime", "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = BATnew(b->htype, TYPE_daytime, BATcount(b));
	if( dst == NULL){
		BBPreleaseref(b->batCacheid);
		throw(SQL, "sql.daytime", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATloop(b,p,q) {
		char *v = (char*)BUNtail(bi,p);
		union {
			lng l;
			daytime r;
		} u;
		msg = str_2time_daytime( &u.r, &v, digits );
		if (msg)
			break;
		BUNins(dst, BUNhead(bi,p), &u.r, FALSE);
	}
	BBPkeepref( *res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}




str
batdaytime_2time_daytime( int *res, int *bid, int *digits )
{
	BAT *b, *dst;
	BATiter bi;
	BUN p,q;
	char *msg = NULL;

	if( (b = BATdescriptor(*bid)) == NULL ){
		throw(SQL, "batcalc.daytime_2time_daytime", "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = BATnew(b->htype, TYPE_daytime, BATcount(b));
	if( dst == NULL){
		BBPreleaseref(b->batCacheid);
		throw(SQL, "sql.daytime", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATloop(b,p,q) {
		daytime *v = (daytime*)BUNtail(bi,p);
		union {
			lng l;
			daytime r;
		} u;
		msg = daytime_2time_daytime( &u.r, v, digits );
		if (msg)
			break;
		BUNins(dst, BUNhead(bi,p), &u.r, FALSE);
	}
	BBPkeepref( *res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}




str
batnil_2time_daytime( int *res, int *bid, int *digits )
{
	BAT *b, *dst;
	BATiter bi;
	BUN p,q;
	char *msg = NULL;

	if( (b = BATdescriptor(*bid)) == NULL ){
		throw(SQL, "batcalc.nil_2time_daytime", "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = BATnew(b->htype, TYPE_daytime, BATcount(b));
	if( dst == NULL){
		BBPreleaseref(b->batCacheid);
		throw(SQL, "sql.daytime", MAL_MALLOC_FAIL);
	}
	BATseqbase(dst, b->hseqbase);
	BATloop(b,p,q) {
		void *v = (void*)BUNtail(bi,p);
		union {
			lng l;
			daytime r;
		} u;
		msg = nil_2time_daytime( &u.r, v, digits );
		if (msg)
			break;
		BUNins(dst, BUNhead(bi,p), &u.r, FALSE);
	}
	BBPkeepref( *res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}


