%{
char rcsid_gram[] = "$Id$";

#include "monetdb_config.h"
#include <stdio.h>
#include "b.h"
#include "fe.h"

extern void doGram(List);
%}

%union {
	int y_int;
	char *y_string;
	Arity y_arity;
	Binding y_binding;
	PatternAST y_patternAST;
	RuleAST y_ruleAST;
	List y_list;
	IntList y_intlist;
}

%start full

%token brgERROR
%token K_TERM
%token K_GRAM
%token K_START
%token K_PPERCENT

%token <y_string> ID
%token <y_int> brgINT

%type <y_arity> decl
%type <y_binding> binding
%type <y_intlist> cost costtail
%type <y_ruleAST> rule
%type <y_patternAST> pattern
%type <y_list> decls rules bindinglist grammarlist
%%


full	: spec
	| spec K_PPERCENT
		{ yyfinished(); }
	;

spec	: decls K_PPERCENT rules
		 { doSpec($1, $3); }
	;

decls	: /* lambda */	 { $$ = 0; }
	| decls decl	 { $$ = newList($2, $1); }
	;

decl	: K_TERM bindinglist	 { $$ = newArity(-1, $2); }
	| K_GRAM grammarlist	 { $$ = 0; doGram($2); }
	| K_START ID		 { $$ = 0; doStart($2); }	/* kludge */
	;

grammarlist	: /* lambda */		 { $$ = 0; }
		| grammarlist ID	 { $$ = newList($2, $1); }
		;

bindinglist	: /* lambda */		 { $$ = 0; }
		| bindinglist binding	 { $$ = newList($2, $1); }
		;

binding	: ID '=' brgINT	 { $$ = newBinding($1, $3); }
	;

rules	: /* lambda */	 { $$ = 0; }
	| rules rule	 { $$ = newList($2, $1); }
	;

rule	: ID ':' pattern '=' brgINT cost ';'	 { $$ = newRuleAST($1, $3, $5, $6); }
		;

pattern	: ID 					 { $$ = newPatternAST($1, 0); }
	| ID '(' pattern ')'			 { $$ = newPatternAST($1, newList($3,0)); }
	| ID '(' pattern ',' pattern ')'	 { $$ = newPatternAST($1, newList($3, newList($5, 0))); }
	;

cost	: /* lambda */		 { $$ = 0; }
	| '(' brgINT costtail ')'	 { $$ = newIntList($2, $3); }
	;

costtail	: /* lambda */		 { $$ = 0; }
		| ',' brgINT costtail	 { $$ = newIntList($2, $3); }
		| brgINT costtail		 { $$ = newIntList($1, $2); }
		;
