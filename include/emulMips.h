

#ifndef EMULMIPS_H
#define EMULMIPS_H

#include "mem.h"
#include "commande.h"

/* prompt du mode shell interactif */
#define PROMPT_STRING "MipsShell : > "
//#define UC(c)
/* taille max pour nos chaines de char */
#define MAX_STR 1024


/*************************************************************\
Valeur de retour speciales lorsqu'on pour la fonction
	int parse_and_execute_cmd_string(char *input) ;
Toute autre valeur signifie qu'une erreur est survenue
 \*************************************************************/
#define CMD_OK_RETURN_VALUE 0
#define CMD_EXIT_RETURN_VALUE -1
#define CMD_UNKOWN_RETURN_VALUE -2
#define CMD_REQUIRE_ARG -3
#define CMD_WRONG_ARG -4
#define NO_ELF_LOAD -5


/* type de token (exemple) */
enum {HEXA,REGISTRE,RANGE,ADD,PLUS,DEUXPOINTS,UNKNOWN};

/* mode d'interaction avec l'interpreteur (exemple)*/
typedef enum {INTERACTIF,SCRIPT,DEBUG_MODE} inter_mode;

/* structure passée en parametre qui contient la connaissance de l'état de
 * l'interpréteur 
 */
typedef struct {
    inter_mode mode;
    char input[MAX_STR];
    char * from;
    char first_token;
} *interpreteur;


typedef struct bp_t{
	struct bp_t *suiv;
	int type;
	vaddr addr;


} *bp;


char* get_next_token(interpreteur inter);
int get_type(char* chaine);
int execute_cmd(interpreteur inter, registre r,mem* memory,bp *bpi);
char* itoa(int val, int base);

#endif
