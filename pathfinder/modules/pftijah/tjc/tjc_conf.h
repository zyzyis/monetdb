#ifndef TJC_CONF_H
#define TJC_CONF_H

#define TJCmalloc GDKmalloc
#define TJCfree   GDKfree

#define MAXMILSIZE      32000

typedef struct tjc_config {
	char* dotFile;
	char* milFile;
	short debug;
	char timing;
	char* ftindex;
	char* irmodel;
	char* orcomb;
	char* andcomb;
	char* upprop;
	char* downprop;
	char* prior;
	double scorebase;
	double lambda;
	double okapik1;
	double okapib;
	char returnall;
	char rmoverlap;
	char errBUFF[1024];
	char milBUFF[MAXMILSIZE];
	char dotBUFF[MAXMILSIZE];
} tjc_config;


#define TJCPRINTF sprintf
#define MILOUT    &(tjc_c->milBUFF[strlen(tjc_c->milBUFF)])
#define DOTOUT    &(tjc_c->dotBUFF[strlen(tjc_c->dotBUFF)])

extern tjc_config* tjc_c_GLOBAL;

#endif
