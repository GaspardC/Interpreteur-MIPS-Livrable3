
#include <stdlib.h>

typedef struct fsm_t {
	char *scripte ;
	FILE *fp;
	dsm_rval code;
	string rval_info;
} *fsm_t


typedef struct { 
	cpv mips;
	Vmem Vmem;
	
} Vm;

typedef struct {
	size_t nregs;
	reg *regs;
} cpu; //emulateur 

typedef unint32_t reg; //registre
typedef unint32_t Vaddr; //adrresse virtuelle sur 32 bits

/*MEMOIRE*/

typedef struct {
	Vaddr addr;
	size_t sz;
	void *ptr;
} seg;

typedef struct {
	size_t nseg;
	seg *segs;
} Vmem;






