
/* The Monet Application Programming Interface
Author M.L. Kersten

The easiest way to extend the functionality of Monet is to construct
an independent Monet application, which communicates with a
running server on a textual basis. 
The prime advantage is a secure and debugable interface. 
The overhead in command preparation and response decoding is mainly 
localized in the workstation of the application.

The Java code base presented here is a 'copy' of the similar C-Mapi interface.
The main differences are that this version does not provide variable
binding to ease interaction.
It is not possible to bind source/destination variables.
Instead, column values should be explicitly extracted from the row cache
and arguments are passed upon query construction.
Furthermore, it uses a simple line-based protocol with the server.

See the corresponding documentation. A synopsis is given only.

bind()     	&       Bind string C-variable to field	 [NON_JAVA]
bind_var() 	&       Bind typed C-variable to field [NON_JAVA]
bind_numeric()	& 	Bind numeric C-variable to field [NON_JAVA]
cacheLimit()	& 	Set the tuple cache limit 
connect()  	&       Connect to a Mserver 
connectSSL()	& 	Connect to a Mserver using SSL [TODO]
disconnect()	& 	Disconnect from server
dup()      	&       Duplicate the connection structure
explain()  	&       Display error message and context on stream
execute()  	&       Execute a query
executeArray()	&	Execute a query using string arguments
fetchField()	&	Fetch a field from the current row
fetchFieldArray()        & Fetch all fields from the current row
fetchLine()	&       Retrieve the next line
fetchReset()	&       Set the cache reader to the begining
fetchRow()	&       Fetch row of values
fetchAllRows()	&       Fetch all answers from server into cache
finish()	&       Terminate the current query
getHost() 	&       Host name of server
getLanguage()	& 	Query language name
getVersion()	& 	Monet version identifier
getVersionId()	& 	Monet version generation number
getUser() 	&       Get current user name
getDBname() 	&       Get current database name
getTable()    	&       Get current table name
getFieldCount()	& 	Number of columns in current row
getRowCount() 	&       Number of rows in cache or -1
gotError()    	&       Test for error occurrence
ping()     	&       Test server for accessibility
prepare()  	&       Prepare a query for execution
prepareArray()	&       Prepare a query for execution using arguments
query()    	&       Send a query for execution
queryArray()	& 	Send a query for execution with arguments
quickQuery()	&       Send a query for execution
quickQueryArray()        & Send a query for execution with arguments
quote()    	& 	Escape characters
rows_affected() & 	Obtain number of rows changed
quickResponse()	&       Quick pass response to stream
seekRow() 	&       Move row reader to specific location in cache
timeout()  	&       Set timeout for long-running queries[TODO]
trace()    	&       Set trace flag
traceLog()	& 	Keep log of interaction
unquote()  	&       remove escaped characters

/*
 * The contents of this file are subject to the Monet Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at 
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Monet Database System.
 * 
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2003 CWI.  
 * All Rights Reserved.
 * 
 * Contributor(s): Martin Kersten <Martin.Kersten@cwi.nl>
 */

import java.awt.*;
import java.applet.Applet;
import java.applet.*;
import java.io.*;
import java.net.Socket;



public class Mapi 
{

	public final static int MOK = 0;
	public final static int MERROR = -1;
	public final static int MEOS = -1;
	public final static int MAPIERROR  = -4;
	public final static char PROMPTBEG= (char) 1;
	public final static char PROMPTEND= (char) 2;

        static final int MAPIBLKSIZE = 1024;


//	public final int SQL_AUTO        =0;      /* automatic type detection */
//	public final int SQL_INT         =1;
//	public final int SQL_LONG        =2;
//	public final int SQL_CHAR        =3;
//	public final int SQL_VARCHAR     =4;
//	public final int SQL_FLOAT       =5;
//	public final int SQL_DOUBLE      =6;
//	public final int SQL_TIME        =7;
//	public final int SQL_DATE        =8;
//	public final int SQL_NUMERIC     =9;

	public final int LANG_MAL        =0;
	public final int LANG_MIL        =1;
	public final int LANG_SQL        =2;
	public final int LANG_XCORE      =3;
	public final int LANG_XQUERY     =4;


	public final String PLACEHOLDER    ="?";


	private String server;
	private boolean MONET5;
	private String host = "localhost";
	private int port = 50001;
	private String username= "anonymous";
	private String password="";
	private String language= "mil";
	private int languageId= LANG_MAL;
	private String dbname= "unknown";
	
	private boolean everything = false;
	private boolean trace = false;
	private boolean connected= false;
	private boolean active= false;
	private boolean eos = false;

	// CacheBlock cache;
	IoCache blk= new IoCache();
    
	private int error = MOK;
	private String errortext="";
	private String action="";
	
	private String query;
	private String qrytemplate;
	private String prompt;
	
	// Tabular cache
	private int fieldcnt= 0;
	private int maxfields= 0;
	private int minfields= 0;
	private int rows_affected =0;
	private RowCache cache= new RowCache();
	private Columns columns[]= new Columns[32];
	
	private Socket socket;
	private DataOutputStream toMonet;
	//private DataInputStream fromMonet;
	private BufferedReader fromMonet;
	private DataOutputStream traceLog;
	

/* 
The static method hostname gets the hostname from the monetport. The
monetport is defined by 'hostname:portnr'. The static method
portnr will return the port number in this string.
*/
public String hostname( String monetport ){
	int pos = monetport.indexOf(':');
	if (pos <= 1) return host;
	return host= monetport.substring( 0,pos);
}

public int portnr( String monetport ){
	int pos = monetport.indexOf(':');
	if (pos >= 0 && pos < monetport.length()){
		return port=Integer.parseInt(monetport.substring( pos+1 ));
	} 
	return port;
}
	
public Mapi(){}
public Mapi( String host, int port, String user, String pwd, String lang)
throws MapiException
{
	connect(host,port,user,pwd,lang);
}
	
//------------------ The actual implementation ---------
/**
 * Toggle the debugging flag for the Mapi interface library
 * @param flg the new value 
*/
public void trace(boolean flg){
	trace= flg;
}

/**
 * Open a file to keep a log of the interaction between client and server.
 * @param fnme the file name
*/
public void traceLog(String fnme){
	System.out.println("setTraceLog not yet implemented");
}

/**
 * This method can be used to test for the lifelyness of the database server.
 * The ping query depends on the language scenario set by the user.
 * @return the boolean indicating success of failure
*/
public boolean ping(){
	//if( language.equals("mal")) query("print(1);");
	return true;
}

/**
 * This method can be used to set the Mapi descriptor once an error
 * has been detected.
 * @return the MAPI internal success/failure indicator
*/
public int setError(String msg, String act){
	errortext= msg;
	error= MERROR;
	action= act;
	return MERROR;
}

/**
 * This method can be used to clear the Mapi error descriptor properties.
*/
public void clrError(){
	errortext="";
	error= MOK;
	action="";
}
/**
 * This method can be used to connect to a running server.
 * The user can indicate any of the supported query languages, e.g.
 * MIL, MAL, or SQL.
 * The method throws an exception when a basic IO error is detected.
 * @param host - a hostname, e.g. monetdb.cwi.nl
 *        port - the port to which the server listens, e.g. 50001
 *        user - user name, defaults to "anonymous"
 *        pwd  - password, defaults to none
 *        lang - language interpretation required
*/
public Mapi connect( String host, int port, String user, String pwd, String lang )
throws MapiException
{
	error = MOK;
	try{
	    if(trace) System.out.println("setup socket connection");
		socket   = new Socket( host, port );
		fromMonet= new BufferedReader(
			new InputStreamReader(socket.getInputStream()));
		//fromMonet= new DataInputStream(socket.getInputStream());
		toMonet= new DataOutputStream(socket.getOutputStream());
		connected = true;
	} catch(IOException e) {
		error= MERROR;
		errortext= "Failed to establish contact\n" ;
		throw new MapiException( errortext );
	}
	this.username= user;
	this.password = pwd;
	this.active = true;
	toMonet(user+":"+pwd+"\n");
	while( !gotError() && active){
		getRow();
		if( server == null) server= blk.buf;
		if( gotError()) return this;
	}
	// Assume return of a single line with the server identity
	MONET5= server.indexOf("5.0")==0;
	if(trace)
		System.out.println("Monet version 5?"+MONET5+":"+
			(server.indexOf("5.0"))+":"+server);
	if(trace) System.out.println("Connection established");
	connected= true;

	// set the language scenario
	if( !language.equals("mil") &&
	    !(language.equals("mal") && MONET5) &&
	    !language.equals("sql") ){
		setError("Illegal language scenario","connect");
		explain();
	} else language= lang;
	// sent the language scenario initialization command
	// query(lang+"()");
	return this;
}

/**
 * Secure communication channels require SSL.
 * @param host - a hostname, e.g. monetdb.cwi.nl
 *        port - the port to which the server listens, e.g. 50001
 *        user - user name, defaults to "anonymous"
 *        pwd  - password, defaults to none
 *        lang - language interpretation required
*/
public static Mapi connectSSL( String host, int port, String user, String pwd, String lang )
{
	System.out.println("connectSSL not yet implemented");
	return null;
}

/**
 * The user can create multiple Mapi objects to keep pre-canned queries around.
 * @param src the Mapi connection to be duplicated
 * @return a duplicate connection
*/
Mapi dup(Mapi j){
	Mapi m= new Mapi();
	m.socket = j.socket;
	m.fromMonet = j.fromMonet;
	m.toMonet = j.toMonet;
	return m;
}

/**
 * The interaction protocol with the monet database server is very
 * simple. Each command (query) is ended with '\n'. The synchronization
 * string is recognized by brackets '\1' and '\2'.
*/
private void promptMonet() throws MapiException  {	
	// last line read is in the buffer
	int lim = blk.buf.length();
	
	prompt= blk.buf.substring(1,lim-1);
	// finding the prompt indicates end of a query
	active= false;
	if( trace) System.out.println("promptText:"+prompt);
    }

/**
 * Disconnect from Monet server.
 * The server will notice a broken pipe and terminate.
 * The intermediate structurs will be cleared.
*/
public void disconnect() {
	for(int i=0; i< cache.writer; i++){
		cache.rows[i]= null;
		cache.fields[i]= null;
	}
	cache.writer= 0;
	cache.reader= -1;
	blk= new IoCache();
	connected= active= false;
	toMonet = null; fromMonet = null; traceLog = null;
}
/**
 * Eat away all the remaining lines
*/
public int finish(){
	try{
		while(active && fetchLine() != null);
	} catch( MapiException e) {}
	return MOK;
}

//======================= Binding parameters ================

private boolean checkColumns(int fnr, String action){
	if( fnr<0){
		setError("Illegal field nummer:"+fnr,action);
		return false;
	}
	if( fnr >= fieldcnt) extendColumns(fnr);
	return true;
}
/**
 * Binding an output parameter to an object simplifies transfer,
 * values are passed to the receiver using the method setValue();
 * @param fnr a valid field index
 * obj a reference to an arbitrary receiving object
*/
public int bind(int fnr, Object o){
	if( !checkColumns(fnr,"bind")) return MERROR;
	System.out.println("bind() yet supported");
	//columns[fnr].outparam = o;
	return MOK;
}
/**
 * Binding an input parameter to an object simplifies transfer,
 * values are passed to the receiver using the method setValue();
 * @param fnr a valid field index
 * obj a reference to an arbitrary receiving object
*/
public int bindVar(int fnr, int tpe, Object o){
	if( !checkColumns(fnr,"bindVar")) return MERROR;
	System.out.println("bindVar() not supported");
	//columns[fnr].outparam = o;
	return MOK;
}

/**
 * Binding an input parameter to an object simplifies transfer,
 * values are passed to the receiver using the method setValue();
 * @param fnr a valid field index
 * scale
 * precision
 * obj a reference to an arbitrary receiving object
*/
public int bindNumeric(int fnr, int scale, int prec, Object o){
	if( !checkColumns(fnr,"bindVar")) return MERROR;
	System.out.println("bindVar() not supported");
	columns[fnr].scale = scale;
	columns[fnr].precision = prec;
	//columns[fnr].outparam = o;
	return MOK;
}

// ------------------------ Columns handling
/**
 * Columns management realizes the tabular display structure envisioned.
*/
private int extendColumns(int mf){
	int nm= maxfields+32;
	if( nm <= mf)
		nm= mf+32;
	if( trace) System.out.println("extendColumns:"+nm);
	Columns nf[]= new Columns[nm];
	System.arraycopy(columns,0,nf,0,maxfields);
	columns= nf;
	maxfields= nm;
	return MOK;
}

private void extendFields(){
	int cr= cache.reader;
	int k= cache.fldcnt[cr];
	if( trace) System.out.println("extendFields:"+cr+","+k);
	if(cache.fields== null || maxfields >k){
		String anew[]= new String[maxfields];
		if( cache.fields != null && cache.fields[cr]!= null)
			System.arraycopy(cache.fields[cr],0,anew,0,k);
		cache.fields[cr]= anew;
	}
}

/** 
 * The values of a row are delivered to any bounded variable before
 * fetch_row returns. Automatic conversion based on common types
 * simplifies the work for the programmers.
*/
private void storeBind(){
	System.out.print("storeBind() Not supported");
	//int cr= cache.reader;
	//for(int i=0; i< fieldcnt; i++)
	//if( columns[i].outparam != null){
		//columns[i].outparam.setValue(s);
	//} 
}

/**
 * The slice row constructs a list of string field values.
 * It trims the string quotes but nothing else.
*/
private String[] sliceRow(){
	int cr= cache.reader;
	char p[]= cache.rows[cr].toCharArray();
	if( p == null)
		setError("Current row missing","sliceRow");
	if( cache.fldcnt[cr]>0){
		storeBind();
		return cache.fields[cr];
	}
	boolean single = p[0]!= '[';
	int i= single? 1:0;
	int f=1,l;
	do{
		if(i== maxfields){
			if( extendColumns(maxfields+32) != MOK)
				return null;
			extendFields();
		}
		//l= unquote(p,f);
		f=l=1;
		//cache.fields[cr]= f;
		while(l<p.length )
		if( p[l]=='\t' || p[l] ==']') break; else l++;
		f= l+1;
	} while( !single && p[i]!=']');
	if( fieldcnt<i) fieldcnt=i;
	storeBind();
	return null;
}

/**
 * The fetchRow() retrieves a tuple from the server
*/
public String[] fetchRow(){
	if( getRow()==MOK) return sliceRow();
	return null;
}
public int fetchAllRows(){
	cacheLimit(-1);
	cache.tuples=0;
	while( getRow()== MOK)
		sliceRow();
	fetchReset();
	return cache.tuples;
}

public String fetchField(int fnr){
	int cr= cache.reader;
	if( fnr>=0){
		if( cache.fldcnt[cr]==0)
			sliceRow();
		//if( fnr < cache.fldcnt[cr])
			//return cache.flds[cr][fnr];
	}
	setError("Illegal field","fetchField");
	return null;
}
public String[] fetchFieldArray(int fnr){
	System.out.println("Not yet implemented");
	return null;
}
public int getFieldCount(){
	return fieldcnt;
}
public int getRowCount(){
	return rows_affected;
}
public String getName(int fnr){
	int cr= cache.reader;
	if( fnr >=0 && fnr < cache.fldcnt[cr]){
		if(columns[fnr].columnname== null)
			columns[fnr].columnname= "column_"+fnr;
		return columns[fnr].columnname;
	}
	setError("Illegal field","getName");
	return null;
}
public String getTable(int fnr){
	int cr= cache.reader;
	if( fnr >=0 && fnr < cache.fldcnt[cr]){
		if(columns[fnr].tablename== null)
			columns[fnr].tablename= "table_"+fnr;
		return columns[fnr].tablename;
	}
	setError("Illegal field","getName");
	return null;
}
public int rows_affected(){
	return rows_affected;
}
public String getHost(){
	return host;
}
public String getUser(){
	return username;
}
public String getLanguage(){
	return language;
}
public String getDBname(){
	return language;
}
public String getVersion(){
	return server;
}
public int getVersionId(){
	if( MONET5) return 5;
	return 4;
}
// ------------------------ Handling queries
/**
 * The routine mapi_check_query appends the semicolon and new line if needed.
 * Furthermore, it ensures that no 'false' newlines are sent to the server,
 * because this may lead to a synchronization error easily.
*/
private void expandQuery(String xtra){
	String n= query+xtra;
	query = n;
	if( qrytemplate != null) qrytemplate= n;
	if( trace) System.out.print("Modified query:"+query);
}

private void checkQuery(){
	clrError();
	int i = query.indexOf('\n');
	if( i>=0 && i < query.length()-1)
		setError("Newline in query string not allowed","checkQuery");
	query.replace('\n',' ');
	if( languageId == LANG_SQL){
		i= query.lastIndexOf(';');
		if( i != query.length()-1) expandQuery(";");
	}
	expandQuery("\n");
}

private int prepareQuery(String cmd){
	if( cmd == null || cmd.length()==0){
		// use previous query
	} else {
		query = cmd;
		qrytemplate= null;
		int i= cmd.indexOf(PLACEHOLDER);
		if( i>=0 && i < cmd.length())
			qrytemplate = query;
	}
	checkQuery();
	return error;
}

/**
 * Move the query to the connection structure. Possibly interact with the
 * back-end to prepare the query for execution.
 * @param cmd the command string to be executed
*/

public int prepare(String cmd){
	return prepareQuery(cmd);
}

/**
 * Move the query to the connection structure. Possibly interact with the
 * back-end to prepare the query for execution.
 * @param cmd the command string to be executed
 *   arg replacement strings for each of the placeholders
*/
public int prepareQueryArray(String cmd, String arg[]){
	int ret= prepareQuery(cmd);
	if( ret != MOK) return ret;

	// get rid of the previous field bindings
	for(int i=0; i< fieldcnt; i++) {
		//columns[i].inparam= null;
	}
	int lim= arg.length;
	if( lim > fieldcnt)
		extendColumns(lim);
	for(int i=0; i< lim; i++) {
		//columns[i].inparam= arg[i];
	}
	return error;
}

/**
 * Before we ship a query, all placeholders should be removed
 * and the actual values should take their position.
 * Replacement should be able to hangle PLACEHOLDERS in the arguments.
*/
private void paramStore(){
	if(qrytemplate == null || qrytemplate.length()==0) return;
	query = qrytemplate;	

	int p, pr=0;
	for(int i=0; i< fieldcnt; i++){
		String left,right;
		p= query.indexOf(PLACEHOLDER,pr);
		if( p == pr){
			// no more placeholders
			setError("Not enough placeholders","paramStore");
			break;
		}
		left = query.substring(0,p-1);
		right= query.substring(p+1,query.length());
		//query= left+columns[i].inparam.toString()+right;
	}
	if( trace) System.out.println("paramStore:"+query);
}

/**
 * The command is shipped to the backend for execution. A single answer
 * is pre-fetched to detect any runtime error. A NULL command is
 * interpreted as taking the previous query. MOK is returned upon success.
 * The fake queries '#trace on' and '#trace off' provide an easy means
 * to toggle the trace flag.
*/
public int execute(){
	paramStore();
	cacheReset();
	if(trace) System.out.print("execute:"+query);
	if( query.indexOf("#trace on")==0){
		System.out.println("Set trace on");
		trace= true;
	}
	if( query.indexOf("#trace off")==0){
		System.out.println("Set trace off");
		trace= false;
	}
	try{
		toMonet(query);
	} catch( MapiException m){
		setError("Write error on stream","execute");
	}	
	return error;
}
/**
 * The command is shipped to the backend for execution. A single answer
 * is pre-fetched to detect any runtime error. A NULL command is
 * interpreted as taking the previous query. MOK is returned upon success.
 * @param arg the list of place holder values
*/
public int executeArray(String arg[]){
	prepareQueryArray(query,arg);
	return error==MOK ? execute(): error;
}

/**
 * This routine sends the Command to the database server.
 * It is one of the most common operations.
 * If Command is null it takes the last query string kept around.
 * It returns zero upon success, otherwise it indicates a failure of the request.
 * The command response is buffered for consumption, e.g. fetchRow();
 * @param cmd - the actual command to be executed

*/
public int query(String cmd){
	prepareQuery(cmd);
	if( error== MOK) execute();
	if( error== MOK){
		// look ahead to detect errors
		int oldrd= cache.reader;
		int rd=0;
		while(error==MOK && active &&rd++< cache.limit)
			getRow();
		cache.reader= oldrd;
	}
	if(trace  && error!=MOK) System.out.println("query returns error");
	return error;
}

/**
 * This routine sends the Command to the database server.
 * It is one of the most common operations.
 * If Command is null it takes the last query string kept around.
 * It returns zero upon success, otherwise it indicates a failure of the request.
 * The command response is buffered for consumption, e.g. fetchRow();
 * @param cmd - the actual command to be executed
   arg - the place holder values

*/
public int queryArray(String cmd, String arg[]){
	prepareQueryArray(cmd,arg);
	if( error== MOK) execute();
	if( error== MOK){
		// look ahead to detect errors
		int oldrd= cache.reader;
		int rd=0;
		while(error==MOK && active &&rd++< cache.limit)
			getRow();
		cache.reader= oldrd;
	}
	if(trace && error !=MOK) System.out.println("query returns error");
	return error;
}

/**
 * To speed up interaction with a terminal front-end,
 * the user can issue the quick_*() variants.
 * They will not analyse the result for errors or
 * header information, but simply through the output
 * received from the server to the stream indicated.
*/
public int quickQuery(String cmd, DataOutputStream fd){
	prepareQuery(cmd);
	if( error== MOK) execute();
	if( error== MOK)
		quickResponse(fd);
	if(trace && error !=MOK) System.out.println("query returns error");
	return error;
}
public int quickQueryArray(String cmd, String arg[], DataOutputStream fd){
	prepareQueryArray(cmd,arg);
	if( error== MOK) execute();
	if( error== MOK)
		quickResponse(fd);
	if(trace && error !=MOK) System.out.println("query returns error");
	return error;
}

// -------------------- Cache Management ------------------
private void cacheReset()
{
	rows_affected= 0;
	active= true;
	for(int i=0; i< cache.writer; i++){
		cache.rows[i]= null;
		cache.fields[i]= null;
	}
	cache.writer=0;
	cache.reader= -1;
	cache.tuples= 0;
	cache.fldcnt = new int[cache.limit];
}

/**
 * Users may change the cache size limit
 * @param limit - new limit to be obeyed
*/
public int cacheLimit(int limit){
	cache.rowlimit= limit;
	return MOK;
}

/**
 * Reset the row pointer to the first line in the cache.
 * This need not be a tuple.
 * This is mostly used in combination with fetching all tuples at once.
 */
public int fetchReset(){
	cache.reader = -1;
	return MOK;
}

/**
 * Reset the row pointer to the requested row number;
 * Comments lines are ignored.
 * @param rownr - the row of interest
 */
public int seekRow(int rownr){
	int i;
	cache.reader= -1;
	if( rownr<0) return setError("Illegal row number","seekRow");
	for(i=0; rownr>=0 && i<cache.writer; i++){
		if( cache.rows[i].indexOf("#")==0 ) continue;
		if( cache.rows[i].indexOf("!")==0 ) continue;
		if( --rownr <0) break;
	}
	if( rownr>=0) return setError("Row not fount","seekRow");
	cache.reader= i;
	return MOK;
}
/**
 * These are the lowest level operations to retrieve a single line from the 
 * server. If something goes wrong the application may try to skip input to 
 * the next synchronization point.
*/
void extendCache(){
	int oldsize= cache.writer;
	if( oldsize == cache.rowlimit){
		setError("Row cache limit reached","extendCache");
	}
	int incr = oldsize /10;
	int newsize = oldsize +(incr<100? 100: incr);
	if( newsize >cache.rowlimit && cache.rowlimit>0)
	newsize = cache.rowlimit;

	String newrows[]= new String[newsize];
	if(oldsize>0){
		System.arraycopy(cache.rows,0,newrows,0,oldsize);
		cache.rows= newrows;
	}
	    
	// anchor stuff
	// field stuff
}

int clearCache(){
	// remove all left-over fields
	if( cache.reader+2<cache.writer)
		return setError("Cache reset with non-read lines","clearCache");
	cacheReset();
	return MOK;
}
    
// ----------------------- Basic line management ----------------
/**
 * The fetchLine() method is the lowest level of interaction with
 * the server to acquire results. The lines obtained are stored in 
 * a cache for further analysis. 
*/
public String fetchLine() throws MapiException {
	    
	if( cache.writer>=0 && cache.reader+1<cache.writer){
		if( trace) System.out.println("useCachedLine:"+cache.rows[cache.reader+1]);
		return cache.rows[++cache.reader];
	}
	if( ! active) return null;
	error= MOK;
	// get rid of the old buffer space
	if( cache.writer ==cache.limit && clearCache()!= 0) return null;

	// any IO errors detected already
	if( error!= MOK)
		setError("Connection terminated","fetchLine");
	// check if you need to read more blocks to read a line
	blk.eos= false;

	// fetch one more block or line
	int n=0;
	int len= 0;
	String s="";
	if( trace) System.out.println("Start reading from server");
	try {
		blk.buf = fromMonet.readLine();
		if(trace) System.out.println("gotLine:"+blk.buf);
	} catch(IOException e){
		error= Mapi.MAPIERROR;
		errortext= "Communication broken";
		throw new MapiException( errortext);
	}
	if( blk.buf==null || (len= blk.buf.length()) == 0){
		blk.eos= true;	
		setError("Lost connection with server","fetchLine");
		connected= false;
		return null;
	}
	if( blk.buf.charAt(0)== PROMPTBEG){
		promptMonet();
		return null;
	}	    

	// we have at least one complete line in the buffer
        // manage the row cache space first
        if( cache.writer >= cache.limit)
		extendCache();
        cache.rows[cache.writer] = blk.buf;
        cache.writer++;
        return cache.rows[++cache.reader];
}

/**
 * If the answer to a query should be simply passed on towards a client,
 * i.e. a stream, it pays to use the quickResponse() routine.
 * The stream is only checked for occurrence of an error indicator
 * and the prompt.
 * The best way to use this shortcut execution is calling
 * mapi_quick_query(), otherwise we are forced to first
 * empy the row cache.
 * @param fd - the file destination
*/

public int quickResponse(DataOutputStream fd){
	String msg;

	if( fd== null)
		return setError("File destination missing","response");
	try{
		while( active && (msg=fetchLine()) != null)
		try{
			fd.writeBytes(msg.toString());
			fd.writeBytes("\n");
			fd.flush();
		} catch( IOException e){
			throw new MapiException("Can not write to destination" );
		}
	} catch(MapiException e){
	}
	return error;

}

//------------------- Response analysis --------------
private void keepProp(String name, String colname){
}

private void header(String line){
	if( trace) System.out.print("header:"+line);
	int etag= line.indexOf("#");
	if( etag==0 || etag== line.length()) return;

	// analyse the header row, but make sure it is restored in the end.
	String old= cache.rows[cache.reader];
	//cache.rows[cache.reader][0]='[';
	//cache.rows[cache.reader][etag]=']';
	
	//REST LATER
	//sliceRow();
	cache.rows[cache.reader]= old;
}

// Extract the next row from the cache.
public int getRow(){
        String reply= "";
        error = MOK;

        while( active ||cache.reader+1< cache.writer){
		if( active){
			try{  
				reply= fetchLine();
			} catch(MapiException e){ 
				if(trace)
				 System.out.print("Got exception in getRow");
				reply=null;
			}
			if( gotError() || reply == null) return MERROR;
		} else reply = cache.rows[++cache.reader];

		if(trace) System.out.println("getRow:"+reply);
		switch(reply.charAt(0)){
		    case '#': 
			header(reply.toString());
			if( languageId>= LANG_SQL){
				if( reply.charAt(1) != '(') return getRow();
			} else return MOK;
		    case '!': 
			// concatenate error messages
			String olderr= errortext;
			clrError();
			setError(olderr+reply.toString(),"getRow");
			break;
		    case '[': 
			cache.tuples++;
		    default:
			return MOK;
		}
        }
        return MOK;
}
// ------------------------------- Utilities
/**
 * unquoteInternal() squeezes a character array to expand
 * all quoted characters. It also removes leading and trailing blancs.
 * @param msg - the byte array to be 
*/
private int unquoteInternal(char msg[], int first)
{
	int i=0, j=0;
	for(; j< msg.length; j++)
	if( msg[j] != ' ' && msg[j]!= '\t') break;

	if( j< msg.length && (msg[j]=='"' || msg[j]=='\'')){
		// quoted string
	} else {
	}
	return j;
}

public String quote(String msg){
	System.out.println("Not yet implemented");
	return null;
}
public String unquote(String msg){
	System.out.println("Not yet implemented");
	return null;
}

public boolean gotError(){
	return error!=MOK;
}
public String getExplanation(){
    return "Mapi:"+dbname+"\nERROR:"+errortext+
	   "\nACTION:"+action+"\nQUERY:"+query+"\n";
}

public String getPrompt(){
	return prompt;
}
public boolean isConnected(){
    return connected;
}


/* 
The low-level operation toMonet attempts to sent a string to the server.
It assumes that the string is properly terminated and a connection to
the server still exists.
*/
private void toMonet(String msg) throws MapiException {
	if( ! connected)
		throw new MapiException( "Connection was disabled" );

	if( msg== null || msg.equals("")){
		if(trace)
			System.out.println("Attempt to send an empty message");
		return;
	}
	try{
		int size= msg.length();
		toMonet.writeBytes(msg);
		toMonet.flush();
	} catch( IOException e){
		throw new MapiException("Can not write to Monet" );
	}
}

/**
 * The routine timeout can be used to limit the server handling requests.
 * @parem time the timeout in milliseconds
*/
public int timeout(int time){
	System.out.println("timeout() not yet implemented");
	return MOK;
}

/**
 * The routine explain() dumps the status of the latest query to standard out.
*/
public int explain(){
	System.out.println(getExplanation());
	return MOK;
}

class IoCache
{   String  buf="";
    boolean eos;
    public IoCache(){
        eos = false;
    }
}
class RowCache{
    int rowlimit= -1;   /* maximal number of rows to cache */
    int limit=100;      /* current storage space limit */
    int writer= 0;
    int reader= -1;
    int tuples=0;     /* actual tuples in cache */
    int  fldcnt[] = new int[100];   /* actual number of columns in each row */
    String rows[]= new String[100]; /* string representation of rows received */
    String fields[][]= new String[100][];
}
class Columns{
    String tablename;
    String columnname;
    String columntype;
    int  colwidth;
    int  coltype;
    int  precision;
    int  scale;
    int  isnull;

    //Object inparam;	// to application variable 
    //Object outparam;	// both are converted to strings
}
}

