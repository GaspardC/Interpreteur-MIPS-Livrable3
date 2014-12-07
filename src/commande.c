#include "registre.h"
#include "commande.h"
#include "emulMips.h"
#include "mem.h"
#include "common/notify.h"
#include "mipself_test.h"
#include <string.h>
#include "common/bits.h"
#include "elf/elf.h"
#include "elf/syms.h"
#include "lib.h"
#include "asm.h"

#include <stdint.h>


/********************************************\
		Commande load
\********************************************/
int
loadcmd(interpreteur inter, mem * memory, registre r)
{
	char           *section_names[NB_SECTIONS] =
	{TEXT_SECTION_STR, RODATA_SECTION_STR, DATA_SECTION_STR,
		BSS_SECTION_STR
	};
	unsigned int	segment_permissions[NB_SECTIONS] = {R_X, R__, RW_, RW_};
	unsigned int	nsegments;
	int		i = 0,	j = 0;
	unsigned int	type_machine;
	unsigned int	endianness;
	//little ou big endian
		unsigned int	bus_width;
	//32 bits ou 64 bits
		unsigned int	next_segment_start = START_MEM;
	//compteur pour designer le d Ã ©but de la prochaine section
		char           *chaine = NULL;

	if ((chaine = get_next_token(inter)) != NULL) {

		//mem memory;
		stab		symtab = new_stab(0);
		//table des symboles
			FILE * pf_elf;

		if ((pf_elf = fopen(chaine, "r")) == NULL) {
			INFO_MSG("cannot open file %s", chaine);
			return CMD_UNKOWN_RETURN_VALUE;
		}
		if (!assert_elf_file(pf_elf)) {
			ERROR_MSG("file %s is not an ELF file", chaine);
			return CMD_UNKOWN_RETURN_VALUE;
		}
		if ((chaine = get_next_token(inter)) != NULL) {
			uint32_t addr;
			sscanf(chaine,"%x",&addr);
			next_segment_start = ((addr) >> 12) << 12;
		}
		//recuperation des info de l 'architecture
			elf_get_arch_info(pf_elf, &type_machine, &endianness, &bus_width);
		//et des symboles
			elf_load_symtab(pf_elf, bus_width, endianness, &symtab);


		nsegments = get_nsegments(symtab, section_names, NB_SECTIONS);
		nsegments+=1;
		//allouer la memoire virtuelle
			* memory = init_mem(nsegments);

		//Ne pas oublier d 'allouer les differentes sections
			j = 0;
		for (i = 0; i < NB_SECTIONS; i++) {
			if (is_in_symbols(section_names[i], symtab)) {
				elf_load_section_in_memory(pf_elf, *memory, section_names[i],segment_permissions[i],next_segment_start);	
				next_segment_start += (((*memory)->seg[j].size._32 + 0x1000) >> 12) << 12;
				//on arrondit au 1 k supp Ã ©rieur
				//print_segment_raw_content(&(*memory)->seg[j]);
				j++;
			}
		}
		
		//--------- RELOCATION -----------
		for (i = 0; i < nsegments-1; i++) {
			reloc_segment(pf_elf, (*memory)->seg[i], *memory,endianness,symtab);
		}
		//--------------------------------
		
		/* arguments : memomry, scn=STACK_SECTION, RW, unsigned long long add_start =0xff7ff000; 
		RW_=2;
		fill_mem_scn(mem vm, char *name, vsize sz, vaddr start, byte * content);
			*/

		

		byte *ehdr;
		//ehdr= calloc(0x800000,sizeof(uint32_t));
		ehdr = malloc( sizeof( Elf32_Ehdr ) );
		char stack_name[20];
		strcpy(stack_name,".stack");
		attach_scn_to_mem(*memory, stack_name, SCN_ATTR( WIDTH_FROM_EHDR(ehdr) , RW_));
		byte       *section    = NULL;
		//section = malloc( 8388608 );
		section =NULL;
		vsize v;
		uint32_t a =8388608;
		v._32=a;
		vaddr va;
		va._32=0xff7ff000;
		fill_mem_scn(*memory, stack_name, v, va, section );
		r->reg[29]=0xffffeffc;



		del_stab(symtab);
		fclose(pf_elf);

		INFO_MSG("elf file load with success");
		return CMD_OK_RETURN_VALUE;
	}
	INFO_MSG("elf file required");
	return CMD_REQUIRE_ARG;

}


/********************************************\
		Commande disp
\********************************************/
int
dispcmd(interpreteur inter, registre r, mem memory)
{
	char           *token = NULL;


	if (memory == NULL) {
		WARNING_MSG("elf file required !!!\n");
		return NO_ELF_LOAD;
	}
	if ((token = get_next_token(inter)) != NULL) {

		/*** MEMOIRE ***/
		if (strcmp(token, "mem") == 0) {
			int		c = 0;
			while ((token = get_next_token(inter)) != NULL) {

				if (strcmp(token, "map") == 0 && c == 0) {
					//printf("\n------ Fichier ELF \"boucle.o\" : sections lues lors du chargement ------\n");
					printf("Virtual memory map (%d segments)",
					       memory->nseg);
					print_mem(memory);
					return CMD_OK_RETURN_VALUE;
				}
				switch (get_type(token)) {
					int		a = 0;
					int		b = 0;
					char           *saveA;
					char           *saveB;
					int		j         , saveAI, saveBI;

				case HEXA:


					saveA = token;
					//numero du segment
						a = numero_segment(token, memory);

					if (a == -1) {
						WARNING_MSG("memoire virtuelle indisponible !!!");
						return CMD_WRONG_ARG;
					}
					token = get_next_token(inter);
					if (token == NULL) {
						WARNING_MSG("Utiliser la syntaxe 0x0000:0x0000");
						return CMD_WRONG_ARG;
					}
					if (strcmp(token, ":") != 0) {
						WARNING_MSG("Mettre des deux points entre les valeurs des adresses en hexa");
						return CMD_WRONG_ARG;
					}
					token = get_next_token(inter);
					if (token == NULL) {
						WARNING_MSG("Utiliser la syntaxe 0x0000:0x0000");
						return CMD_WRONG_ARG;
					}
					if (get_type(token) == HEXA) {
						saveB = token;
						//numero du segment
							b = numero_segment(token, memory);
						if (b == -1) {
							WARNING_MSG("memoire virtuelle indisponible !!!");
							return CMD_WRONG_ARG;
						}
						if (b != a) {
							WARNING_MSG("Segment differents !!!");
							return CMD_WRONG_ARG;
						}
						sscanf(saveA, "%x", &saveAI);
						sscanf(saveB, "%x", &saveBI);

						if (saveAI > saveBI) {
							WARNING_MSG("Plage d'adresse non conforme !!!");
							return CMD_WRONG_ARG;
						}
						for (j = saveAI; j <= saveBI; j++) {

							if (j % 16 == 0)
								printf("\n0x%08x  ", j);
							printf("%02x ", *(memory->seg[a].content + (j - memory->seg[a].start._64)));
						}
					} else {
						WARNING_MSG("value \"%s\" is not a valid argument of command %s", token, "disp");
						return 1;
					}

					break;
				default:
					WARNING_MSG("value %s is not a valid argument of command %s", token, "disp");
					return CMD_WRONG_ARG;
				}
			}

		}
		/*** REGISTRE ***/
		else if (strcmp(token, "reg") == 0) {
			int		c = 0;

			while ((token = get_next_token(inter)) != NULL) {
				if (strcmp(token, "all") == 0 && c == 0) {
					int		i;
					for (i = 0; i < 35; i++) {
						c++;
						printf("%s : %.8x\t", getName2(i),
						    getRegisterValue(r, i));
						if (c % 4 == 0)
							printf("\n");
					}
					printf("\n");
					return CMD_OK_RETURN_VALUE;
				}
				switch (get_type(token)) {

				case REGISTRE:

					c++;
					printf("%s : %.8x\t", getName(token),
					   getRegisterValueByStr(r, token));
					if (c % 4 == 0)
						printf("\n");
					break;

				default:
					WARNING_MSG("value %s is not a valid argument of command %s", token, "disp");
					return CMD_UNKOWN_RETURN_VALUE;
					break;
				}
			}
		}
		printf("\n");
		return CMD_OK_RETURN_VALUE;
	}
	return CMD_REQUIRE_ARG;
}


/********************************************\
		Commande set
\********************************************/
int
setcmd(interpreteur inter, registre r, mem memory)
{


	char           *token = NULL;
	char           *a = NULL;
	char           *b = NULL;
	int		c = 0;
	if (memory == NULL) {
		WARNING_MSG("elf file required !!!");
		return CMD_WRONG_ARG;
	}
	token = get_next_token(inter);
	if (token == NULL) {
		WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur> \n\"set\" \"reg\" <registre> <valeur>");
		return CMD_WRONG_ARG;
	}
	if (strcmp(token, "reg") != 0 && strcmp(token, "mem") != 0) {
		WARNING_MSG("Structure valide :\n\"set\" \"reg\" <registre> <valeur>");
		return CMD_WRONG_ARG;
	}
	if (strcmp(token, "reg") == 0) {

		token = get_next_token(inter);
		if (token == NULL) {
			WARNING_MSG("Structure valide :\n\"set\" \"reg\" <registre> <valeur>");
			return CMD_WRONG_ARG;
		}
		switch (get_type(token)) {



		case REGISTRE:

			a = strdup(token);
			token = get_next_token(inter);
			if (token == NULL) {
				WARNING_MSG("Structure valide :\n\"set\" \"reg\" <registre> <valeur>");
				return CMD_WRONG_ARG;
			}
			b = strdup(token);
			sscanf(b, "%x", &c);
			setRegisterValue(r, convert(a), c);
			return CMD_OK_RETURN_VALUE;
			break;

		default:

			WARNING_MSG("value %s is not a valid argument of command %s", token, "set");
			return CMD_WRONG_ARG;
			break;
		}
	}
	if (strcmp(token, "mem") == 0) {

		token = get_next_token(inter);

		if ((token) == NULL) {
			WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
			return CMD_WRONG_ARG;
		}
		char           *aa;
		int		n = 0;
		int		bI;
		int		v;

		aa = strdup(token);
		//byte ou word


			if ((strcmp(aa, "byte") != 0) && (strcmp(aa, "word") != 0)) {
			WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
			return CMD_WRONG_ARG;
		}
		/*** byte ***/
		if (strcmp(aa, "byte") == 0) {

			token = get_next_token(inter);
			if ((token) == NULL) {
				WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
				return CMD_WRONG_ARG;
			}
			//printf("token : %s\n", token);
			if (get_type(token) != HEXA) {
				WARNING_MSG("adresse doit Ãªtre donnÃ©e en hexa");
				return CMD_WRONG_ARG;
			}
			sscanf(token, "%x", &bI);

			n = numero_segment(token, memory);
			if (n == -1) {
				WARNING_MSG("memoire virtuelle indisponible !!!");
				return CMD_WRONG_ARG;
			}
			token = get_next_token(inter);

			if ((token) == NULL) {
				WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
				return CMD_WRONG_ARG;
			}
			sscanf(token, "%x", &v);

			if ((int)v > 256) {
				WARNING_MSG("your writed 'byte' but the value is a word");
				return CMD_WRONG_ARG;
			}
			*(memory->seg[n].content + (bI - memory->seg[n].start._64)) = v;
			return CMD_OK_RETURN_VALUE;
		}
		/*** word ***/
		if (strcmp(aa, "word") == 0) {
			token = get_next_token(inter);

			if (token == NULL) {
				WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
				return CMD_WRONG_ARG;
			}
			if (get_type(token) != HEXA) {
				WARNING_MSG("adresse doit Ãªtre donnÃ©e en hexa");
				return CMD_WRONG_ARG;
			}
			sscanf(token, "%x", &bI);

			n = numero_segment(token, memory);
			if (n == -1) {
				WARNING_MSG("memoire virtuelle indisponible !!!");
				return CMD_WRONG_ARG;
			}
			token = get_next_token(inter);

			if (token == NULL) {
				WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
				return CMD_WRONG_ARG;
			}
			sscanf(token, "%x", &v);
			DEBUG_MSG("value %08x\n", v);

			uint32_t	a = v & 0x000000FF;
			uint32_t	b = (v & 0x0000FF00) >> 8;
			uint32_t	c = (v & 0x00FF0000) >> 16;
			uint32_t	d = (v & 0xFF000000) >> 24;

			*(memory->seg[n].content + (bI + 3 - memory->seg[n].start._64)) = a;
			*(memory->seg[n].content + (bI + 2 - memory->seg[n].start._64)) = b;
			*(memory->seg[n].content + (bI + 1 - memory->seg[n].start._64)) = c;
			*(memory->seg[n].content + (bI - memory->seg[n].start._64)) = d;

			return CMD_OK_RETURN_VALUE;
		}
	}
	INFO_MSG("None");
	return CMD_UNKOWN_RETURN_VALUE;
}



/********************************************\
		Commande assert
\********************************************/
int
assertcmd(interpreteur inter, registre r, mem memory)
{
	char           *token = NULL;
	int		a = 0;
	token = get_next_token(inter);
	int		bI;
	//adresse hexa
		int		v;
	//valeur
		int		n;

	if (memory == NULL) {
		printf("load an elf file before\n");
		return CMD_WRONG_ARG;
	}
	if (token == NULL) {
		WARNING_MSG("argument required\n");
		return CMD_REQUIRE_ARG;
	}
	if (strcmp(token, "reg") == 0) {
		if ((token = get_next_token(inter)) != NULL) {
			switch (get_type(token)) {

			case REGISTRE:
				a = getRegisterValue(r, convert(token));
				if ((token = get_next_token(inter)) != NULL) {

					if (a == atoi(token)) {
						//***********------->PROBLEME ! !!Marche pour "assert reg $1 dhcsjd"
							INFO_MSG("Valeurs identiques\n");
						return 0;
					} else {
						INFO_MSG("Valeurs differentes\n");
						return 1;
					}
				} else {
					WARNING_MSG("argument required\n");
					return CMD_REQUIRE_ARG;
				}
				break;

			default:
				WARNING_MSG("register type required\n");
				return CMD_WRONG_ARG;
				break;
			}
		} else {
			WARNING_MSG("argument required\n");
			return CMD_REQUIRE_ARG;
		}

	}
	if (strcmp(token, "byte") == 0) {

		token = get_next_token(inter);

		if (token == NULL)
			return CMD_WRONG_ARG;
		printf("token : %s\n", token);
		if (get_type(token) != HEXA) {
			return CMD_WRONG_ARG;
			DEBUG_MSG("pas hexa");
		}
		sscanf(token, "%x", &bI);
		//adresse en hexa

			n = numero_segment(token, memory);
		token = get_next_token(inter);

		if ((token) == NULL) {
			DEBUG_MSG("pb");
			return CMD_WRONG_ARG;
		}
		sscanf(token, "%x", &v);

		if ((int)v > 256) {
			INFO_MSG("your writed 'byte' but the value is a word");
			return CMD_WRONG_ARG;
		}
		if (v == *(memory->seg[n].content + (bI - memory->seg[n].start._64))) {
			INFO_MSG("Ok");
			return CMD_OK_RETURN_VALUE;
		} else {
			DEBUG_MSG("pas egaux");
			return CMD_WRONG_ARG;
		}

	}
	if (strcmp(token, "word") == 0) {

		token = get_next_token(inter);

		if (token == NULL)
			return CMD_WRONG_ARG;
		printf("token : %s\n", token);
		if (get_type(token) != HEXA) {
			return CMD_WRONG_ARG;
			DEBUG_MSG("pas hexa");
		}
		sscanf(token, "%x", &bI);
		//adresse en hexa

			n = numero_segment(token, memory);
		token = get_next_token(inter);

		if ((token) == NULL) {
			DEBUG_MSG("pb");
			return CMD_WRONG_ARG;
		}
		sscanf(token, "%x", &v);

		uint32_t	combined =
		(*(memory->seg[n].content + (bI - memory->seg[n].start._64))) << 24 |
		(*(memory->seg[n].content + (bI + 1 - memory->seg[n].start._64))) <<
		16 | (*(memory->seg[n].content + (bI + 2 - memory->seg[n].start._64)))
		<< 8 |
		(*(memory->seg[n].content + (bI + 3 - memory->seg[n].start._64)));
		printf("%x\n", combined);
		if (v == combined) {
			INFO_MSG("Ok");
			return CMD_OK_RETURN_VALUE;
		} else {
			DEBUG_MSG("pas egaux");
			return CMD_WRONG_ARG;
		}
	}
	WARNING_MSG("argument required\n");
	return CMD_REQUIRE_ARG;
}

/********************************************\
		Commande debug
\********************************************/

void
debugcmd(interpreteur inter)
{
	inter->mode = INTERACTIF;
}

/********************************************\
		Commande resume
\********************************************/

void
resumecmd(interpreteur inter)
{
	inter->mode = SCRIPT;
}

int
IsInText(mem memory, int add)
{
	uint32_t	u = 0;
	char		s         [20];
	sprintf(s, "%x", add);
	//DEBUG_MSG("s %s", s);
	int		n = numero_segment(s, memory);
	if (n == -1) {
		WARNING_MSG("no in text");
		return -1;
	}
	u = memory->seg[0].size._32;
	uint32_t	v = memory->seg[0].start._32;
	//DEBUG_MSG("size %x start %x add %x", u, v, add);
	if (add - v < u) {
		return 1;
	} else
		return -1;
}

/********************************************\
		Commande disasm
\********************************************/

int
disasmcmd(interpreteur inter, mem memory)
{

	if (memory == NULL) {
		WARNING_MSG("elf file required");
		return CMD_WRONG_ARG;
	}
	char           *token = NULL;
	char           *addA = NULL;
	char           *addB = NULL;
	char           *inc = NULL;
	int		addAI     , addBI;
	int		incI;
	int		n         , m;
	int		b;
	int		i = 0;
	int		combined;

	if ((token = get_next_token(inter)) == NULL) {
		WARNING_MSG("Structure valide :\n \"disasm\" <plage>+");
		return CMD_WRONG_ARG;
	}
	if (get_type(token) != HEXA) {
		WARNING_MSG("not hexa");
		return CMD_WRONG_ARG;
	}
	addA = token;

	if ((token = get_next_token(inter)) == NULL) {
		WARNING_MSG("Structure valide :\n \"disasm\" <plage>+");
		return CMD_WRONG_ARG;
	}
	switch (get_type(token)) {
	case PLUS:
		if ((token = get_next_token(inter)) == NULL) {
			WARNING_MSG("Valeur requise");
			return CMD_WRONG_ARG;
		}
		inc = token;
		sscanf(addA, "%x", &addAI);
		sscanf(inc, "%x", &incI);
		incI = incI - (incI % 4);
		n = numero_segment(addA, memory);
		if (n == -1) {
			INFO_MSG("memory out of bonds");
			return CMD_WRONG_ARG;
		}
		sscanf(addAI + inc, "%x", &b);
		/*
		 * char s[100]; strcat(s,"0x"); strcat(s,(char*)b);
		 * 
		 * if(numero_segment(s,memory)!=0) {DEBUG_MSG("out of .text");
		 * return CMD_WRONG_ARG; }
		 */
		for (i = addAI; i < addAI + incI; i++) {
			if (i - addAI == memory->seg[n].size._32) {
				break;
			}
			if (i % 4 == 0) {
				combined = (*(memory->seg[n].content + (i - memory->seg[n].start._64))) << 24 | (*(memory->seg[n].content + (i + 1 - memory->seg[n].start._64))) << 16 | (*(memory->seg[n].content + (i + 2 - memory->seg[n].start._64))) << 8 | (*(memory->seg[n].content + (i + 3 - memory->seg[n].start._64)));
				printf("%x :: %.8x\t", i, combined);
				dispAssem(combined);
			}
		}
		return CMD_OK_RETURN_VALUE;
		break;

	case DEUXPOINTS:
		if ((token = get_next_token(inter)) == NULL) {
			DEBUG_MSG("Erreur");
			return CMD_WRONG_ARG;
		}
		/* verifier que zone mÃ©moire dans segment */
		addB = token;
		sscanf(addA, "%x", &addAI);
		sscanf(addB, "%x", &addBI);
		n = numero_segment(addA, memory);
		m = numero_segment(addB, memory);

		if (-1 == n) {
			WARNING_MSG("Mauvaise adresse %s", addA);
			return CMD_WRONG_ARG;
		}
		if (-1 == m) {
			WARNING_MSG("Mauvaise adresse %s", addB);
			return CMD_WRONG_ARG;
		}
		if (n != 0 || m != 0) {
			DEBUG_MSG("adresse non dans .text");
			return CMD_WRONG_ARG;
		}
		for (i = addAI; i <= addBI; i++) {
			if (i - addAI == memory->seg[n].size._32) {
				break;
			}
			if (i % 4 == 0) {
				//une instruction tous les 4(= word de 32 bits)
					combined = (*(memory->seg[n].content + (i - memory->seg[n].start._64))) << 24 | (*(memory->seg[n].content + (i + 1 -
																		     memory->seg[n].start._64))) << 16 | (*(memory->seg[n].content + (i + 2 - memory->seg[n].start._64))) << 8 | (*(memory->seg[n].content + (i + 3 - memory->seg[n].start._64)));

				printf("%x :: %.8x\t", i, combined);
				dispAssem(combined);
			}
		}
		return CMD_OK_RETURN_VALUE;
		break;

	default:
		WARNING_MSG("register type required\n");
		return CMD_WRONG_ARG;
		break;
	}

	INFO_MSG("ni : ni + : respercter la syntaxe");
	return CMD_WRONG_ARG;
}
/********************************************\
	Fonction run
\********************************************/


int
run(interpreteur inter, registre r, mem memory, bp bp)
{

	
	if (memory == NULL) {
		WARNING_MSG("elf file required !!!\n");
		return NO_ELF_LOAD;
	}
	//Si Pc n'est pas dans .text on met par défaut PC au début de .text
	if(-1==IsInText(memory,getRegisterValue(r,32))){
	setRegisterValue(r, 32, memory->seg[0].start._32);
	}
	char           *token = NULL;
	int		a = 0;
	char		PC        [100];
	int		b = 1;
	/*
	 * on charge l'adresse si elle est passé en argument et on vérifie
	 * qu'on commence à un mutliple de 4
	 */
	a = getRegisterValue(r, 32);
	DEBUG_MSG("PC %s a %x", PC, a);
	sprintf(PC, "%x", a);
	if ((token = get_next_token(inter)) != NULL) {
		strcpy(PC, token);
		sscanf(PC, "%x", &a);
	}
	//DEBUG_MSG("PC %s", PC);
	sscanf(PC, "%x", &a);
	//DEBUG_MSG("PC %x", a);
	a -= a % 4;
	setRegisterValue(r, 32, a);


	//boucle infini tant qu 'on voit pas un breakpoint ou qu' on est à la fin de.text
		while (b == 1) {
		a = getRegisterValue(r, 32);
		if (-1 == IsInText(memory, a)) {
			DEBUG_MSG("Plus dans .text");
			break;
		}
		if (NULL != check_bp(bp, a)) {
			printf("Breakpoint en %x STOP PC = %x\n", a,a);
			a+=4; //juste pour pouvoir faire un autre run apres le stop
			setRegisterValue(r,32,a);
			break;
		}
		step(inter, r, memory, &b);
	}
	a=getRegisterValue(r,32);

	
	if(IsInText(memory,a)==-1){ //si on est a la fin de .text
	INFO_MSG("run done STOP!");
//On peut remettre Pc a 3000 pour re run apres direct
		setRegisterValue(r, 32, memory->seg[0].start._32);}
	return CMD_OK_RETURN_VALUE;

}

bp
check_bp(bp breakpoint, int PC)
{
	char		PCS       [100];
	sprintf(PCS, "%x", PC);
	bp		bpa = find_by_add(breakpoint, PCS);
	/*
	 * regarder longuer liste initialiser a=0 pour 0 a longuer de la
	 * liste-1 regarder si breakpoint->Vaddr==PC si oui a=1 puis on
	 * return a
	 */
	if (bpa == NULL) {
		DEBUG_MSG("no bp en PC %x", PC);
		return NULL;
	} else {
		DEBUG_MSG("bp en PC %x", PC);
		return bpa->suiv;
	}
}






/********************************************\
	Fonction step
\********************************************/

int
step(interpreteur inter, registre r, mem memory, int *b)
{

	int		n = 0, a = 0;
	char		type      [10];
	strcpy(type, "WORD");
	//adresse
	if (IsInText(memory, r->reg[32]) == -1) {
		WARNING_MSG("Not in seg text  : Pc now initialized to .text start");
		setRegisterValue(r,32,memory->seg[0].start._32); 
		return CMD_WRONG_ARG;
	}
	char		hex       [50];
	sprintf(hex, "%x", r->reg[32]);
	DEBUG_MSG("PC %08x", r->reg[32]);
	n = numero_segment(hex, memory);
	if (-1 == n) {
		WARNING_MSG("Mauvaise adresse %s", hex);
		return CMD_WRONG_ARG;
	}
	uint32_t	u;
	u = loadmem(r->reg[32], memory, type);
	DEBUG_MSG("a %x il y a l'instruction  %32x\n", r->reg[32], u);
	a = execute_asm(u, r, memory);
	//renvois 200 si syscall
		if (syscall(r, memory, a, b) == 10) {
		return 10;
	}
	r->reg[32]+= 4;
	return CMD_OK_RETURN_VALUE;

}

int
syscall(registre r, mem memory, int a, int *b)
{
	//avec 10= 'code syscall' à d é finir
	if (a != 10) {
		return 0;
	}
	int		v0 = 0;
	v0 = getRegisterValue(r, 2);
	if (v0 == 10)
		//exit
	{
		INFO_MSG("Fin d'execution");
		int		PC = 0x3000;
		//pas oblig é mais on se remet au debut de.text // TODO remplacer le 0x3000 par mem->seg[n]->start._32
			setRegisterValue(r, 32, PC);
		*b = 10;
		//ca va couper le while (b == 1)
			return 10;
	}
	if (v0 == 1) {
		//afficher un entier sur la sortie standard
			int		a0 = getRegisterValue(r, 4);
		INFO_MSG("affichage syscall sur l'entree standard");
		printf("%d\n", a0);
		return 1;
	}
	if (v0 == 4) {
		//affichage chaine de caractere
			int		a0 = getRegisterValue(r, 4);
		//TODO aller verifier que on est dans.data puis lire a l 'adresse a0-start de data
			char           *s = NULL;
		INFO_MSG("affichage par adresse de syscall sur l'entree standard");
		printf("%s %d\n", s, a0);
		//j 'ai mis a0 juste pour enlever els Warnings en attendant de le faire propre
			return 4;
	}
	if (8 == v0) {
		//lire une chaine sur l 'entrée standard
			int		a1 = getRegisterValue(r, 5);
		int		a0 = getRegisterValue(r, 4);
		//TODO aller verifier que on est dans.data puis lire a l 'adresse a0-start de data
			char		s         [a1];
		scanf("%s %d %d", s, &a0, &a1);
		//j 'ai mis &a0 et &a1 juste pour enlever les warnings en attendant
			INFO_MSG("Lire une chaine par syscall sur l'entree standard");
		return 8;
	} else
		return 0;
}

int
breakcmd(interpreteur inter, mem memory, bp * bpa)
{
	char           *token = NULL;
	token = get_next_token(inter);
	if (token == NULL) {
		WARNING_MSG("Structure valide :\n\" break\" \"add <adresse>+ or del <adresse>+ |all or list \n");
		return CMD_WRONG_ARG;
	}

	if (memory == NULL) {
		WARNING_MSG("elf file required !!!\n");
		return NO_ELF_LOAD;
	}

	if (0 == strcmp(token, "add")) {
		while ((token = get_next_token(inter)) != NULL) {


			 *bpa = ajouter_en_tete(*bpa, token,memory);
			/*
			 * while((bpb)->suiv!=NULL){ DEBUG_MSG("*bpa suiv %p
			 * vaut %.32x",(*bpa)->suiv,(*bpa)->suiv->addr._32);
			 * bpb=bpb->suiv;}
			 */
		}
		return CMD_OK_RETURN_VALUE;
	}
	if (0 == strcmp(token, "del")) {
		token = get_next_token(inter);

		if (token == NULL) {
			WARNING_MSG("Structure valide  del <adresse>+ |all \n");
			return CMD_WRONG_ARG;
		}
		if (0 == strcmp(token, "all")) {
			/* DEBUG_MSG("free_list avant *bpa %p",*bpa); */
			*bpa = free_list(*bpa);
			/* DEBUG_MSG("free_list apres *bpa %p",*bpa); */

			return CMD_OK_RETURN_VALUE;
		}
		if (get_type(token) == HEXA) {
			do
				*bpa = free_bp(*bpa, token);
			while ((token = get_next_token(inter)) != NULL && get_type(token) == HEXA);
		}
		return CMD_OK_RETURN_VALUE;
	}
	if (*bpa == NULL) {
		DEBUG_MSG("liste break point vide");
	}
	if (0 == strcmp(token, "list")) {
		print_list(*bpa);
		return CMD_OK_RETURN_VALUE;
	}
	return CMD_OK_RETURN_VALUE;
}

bp
free_bp(bp bpa, char *token)
{

	if (bpa == NULL) {
		DEBUG_MSG("liste vide");
		return NULL;
	} else {
		bp		bpb = find_by_add(bpa, token);
		//renvoie celui d 'avant
			if (bpb == NULL) {
			DEBUG_MSG("no matches ");
			return bpa;
		}
		if (NULL == ((bpb->suiv)->suiv) && bpb->suiv != NULL) {
			//on veut supprimer la queue de liste
				free(bpb->suiv);
			if (bpb->addr._32 == 0xFFFFFFFF) {
				//c 'est la queue et la tete de liste 1 seul element
					free(bpb);
				DEBUG_MSG("liste vide");
				return NULL;
			} else {
				//si il ya d 'autre elements avant 
					bpb->suiv = NULL;
				DEBUG_MSG("liste non vide");
				return bpa;
			}
		}
		if (bpb->suiv == bpa) {
			DEBUG_MSG("first match");
			if (bpb->suiv->suiv == NULL) {
				free(bpb->suiv);
				free(bpb);
				return NULL;
			} else {
				bp		bpd = bpb->suiv->suiv;
				DEBUG_MSG("bpd %x", bpd->addr._32);
				free(bpb->suiv);
				free(bpb);
				return bpd;
			}
		} else {
			bp		bpc = bpb->suiv->suiv;
			DEBUG_MSG("on libere %x et on connecte %x a %x", bpb->suiv->addr._32, bpb->addr._32, bpc->addr._32);
			free(bpb->suiv);
			(bpb->suiv) = bpc;
			return bpa;
		}
	}
}
bp
find_by_add(bp bpa, char *token)
{
	uint32_t	a = 0;
	sscanf(token, "%x", &a);
	bp		bpb;
	bpb = bpa;
	if (bpa == NULL) {
		DEBUG_MSG("no breakpoints to match");
		return NULL;
	}
	/*
	 * if(bpa->suiv==NULL) { WARNING_MSG("just 1 breakpoint");
	 * if(bpa->addr._32==a) return bpa; }
	 */

	if (bpa->addr._32 == a) {
		bp		newbp = NULL;
		newbp = malloc(sizeof(*newbp));
		newbp->suiv = bpa;
		newbp->addr._32 = 0xFFFFFFFF;
		DEBUG_MSG("first add match");
		return newbp;
	}
	while (bpb->suiv != NULL) {

		if ((bpb->suiv)->addr._32 == a) {
			DEBUG_MSG("addresse matched %32.x", (bpb->suiv)->addr._32);
			return bpb;
		} //on revoie celui d 'avant
			bpb = bpb->suiv;
	}
	DEBUG_MSG("No matches found");
	return NULL;
}
bp
free_list(bp bpa)
{
	while (bpa != NULL) {
		DEBUG_MSG("bpa %p", bpa);
		bp		bpb = bpa;
		free(bpa);
		bpa = bpb->suiv;
	}
	return NULL;
}

int
deja_dans_liste(bp bp0, int a)
{
	if (bp0 == NULL) {
		//DEBUG_MSG("vide");
		return 1;
	}
	if (bp0->suiv == NULL && bp0->addr._32 == a) {
		INFO_MSG("Breakpoint existe deja a cette adresse");
		return 0;
	}
	while (bp0->suiv != NULL) {
		if (a == bp0->addr._32) {
			INFO_MSG("Breakpoint existe deja a cette adresse");
			return 0;
		}
		bp0 = bp0->suiv;
	}
	//DEBUG_MSG("non vide mais n'existe pas deja");
	return 1;
}

bp
ajouter_en_tete(bp bp0, char *token, mem memory)
{
	unsigned int	a = 0;
	sscanf(token, "%x", &a);
	a = a - a % 4;
	//On met des breakpoints que sur des multiples de 4
		DEBUG_MSG("adresse du bp %x", a);
	if (0 == deja_dans_liste(bp0, a)) {
		return bp0;
	} //si le breakpoint existe déja on ne modifie rien
	if(-1==IsInText(memory,a)){
		return bp0;
	}
	
		bp newbp = calloc(1, sizeof(*newbp));
	

	newbp->addr._32 = a;
	newbp->suiv = bp0;
	/*
	 * DEBUG_MSG("ajout breakpoint %p nouveau  vaut
	 * %x",newbp,newbp->addr._32); if(bp0!=NULL){ DEBUG_MSG("previous
	 * breakpoint %p vaut %x",bp0,(bp0->addr)._32);}
	 */
	return newbp;
}


void
print_list(bp bp0)
{
	bp		bpp = bp0;
	if (bpp == NULL) {
		INFO_MSG("no breakpoints");
	}
	while (bpp != NULL) {
		printf("%x\n", (bpp->addr)._32);
		bpp = bpp->suiv;
	}
}

/********************************************\
	Fonction numero de segment
\********************************************/

int
numero_segment(char *chaine, mem memory)
{
	uint32_t	a = 0,	b = 0, c = 0;
	sscanf(chaine, "%x", &a);
	int		i = 0;
	//i = n numero de segment
		// printf("%x\n", a);
	for (i = 0; i < memory->nseg; i++) {
		b = (uint32_t) memory->seg[i].start._64;
		c = (uint32_t) memory->seg[i + 1].start._64;
		//printf("BORNES SEG%x %x\n", b, c);

		if (b <= a && a < c) {
			return i;
		}
	}
	return -1;
}
