/**
 * @file emulMips.c
 * @author François Cayre, Nicolas Castagné, François Portet
 * @brief Main pour le début du projet émulateur MIPS.
 *
 */
#include "registre.h"
#include "commande.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>


/* la librairie readline */
#include <readline/readline.h>
#include <readline/history.h>

/* macros de DEBUG_MSG fournies , etc */
#include "common/notify.h"

/**
 * allocation et init interpreteur
 * @return un pointeur vers une structure allouée dynamiquement
 */
interpreteur init_inter(void) {
    interpreteur inter = calloc(1,sizeof(*inter));
    if (inter ==NULL)
        ERROR_MSG("impossible d'allouer un nouvel interpreteur");
    return inter;
}

/**
 * desallocation de l'interpreteur
 * @param inter le pointeur vers l'interpreteur à libérer
 */
void del_inter(interpreteur inter) {
    if (inter !=NULL)
        free(inter);
}

/**
 * return le prochain token de la chaine actuellement
 * analysée par l'interpreteur
 * La fonction utilise une variable interne de l'interpreteur
 * pour gérer les premiers appels a strtok
 * @inter le pointeur vers l'interpreteur
 * @return un pointeur vers le token ou NULL
 */
char* get_next_token(interpreteur inter) {

    char       *token = NULL;
    char       *delim = " \t\n";

    if ( inter->first_token == 0 ) {
        token = strtok_r( inter->input, delim, &(inter->from) );
        inter->first_token = 1;
    }
    else {
        token = strtok_r( NULL, delim, &(inter->from) );
    }

    if ( NULL == token ) {
        inter->first_token = 0;
    }

    return token;
}

/**
 * teste si un token est une valeur hexa
 * ATTENTION cette méthode n'est pas complete et ne fonctionnera pas dans tous les cas
 * essayer avec 0x567ZRT...
 *@param chaine le token à analyser
 *@return 0 si non-hexa, non null autrement
 */

/*
int is_hexa_c (char c) {


    if( ( (c>=UC('0')) && (c<=UC('9'))) || (c>='a' && c<='f') || (c>='A' && c<='F') == 1)
    return 1;
    else return 0;

} */

int is_hexa(char* chaine) {
    //int i =0;
    int j;


    if(chaine==NULL || strlen(chaine)<=2 || chaine[0]!='0' || chaine[1]!='x' )   // || sscanf(chaine,"%x",&i)==1) ==0
        return 0;
    else {

        for(j=2; j<strlen(chaine); j++)

        {   //isxdigit retourne 0 si cest faux
            if (isxdigit(chaine[j]) == 0 )

                return 0;
        }

        return 1; // C'est  alors bien un hexa


    }
}

/** entier to hexa***/ //utiliser sscanf

char* itoa(int val, int base) {

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)

        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i+1];

}





/*** EN ATTENDANT UTILISER isxdigit ***/


/**
 * retourne le type du token (fonction très incomplete)
 * @param chaine le token à analyser
 * @return un entier correspondant au type du token
 *
 */
int get_type(char* chaine) {


    if (is_hexa(chaine))
        return HEXA; // HEXA ==1

    if(convert(chaine)!=-1){
        return REGISTRE;
    }

    if(is_hexa(chaine))
        return ADD;
    if(strcmp(chaine,":")==0)
        return DEUXPOINTS;

    if (strcmp(chaine,"+")==0)
        return PLUS;



    return UNKNOWN;
}

/*************************************************************\
Les commandes de l'émulateur.

 Dans cette version, deux commandes :
	"test" qui attend un nombre strictement positifs d'hexa strictement positifs et affiche ce(s) nombre() +1 dans le terminal
	"exit" qui quitte l'émulateur

 \*************************************************************/

/**
 * version de la commande test qui prend en entrée un hexa
 * la fonction verfiie si le nombre est >=0
 * @param hexValue la valeur à afficher
 * @return 0 en case de succes, un nombre positif sinon
 */
int _testcmd(int hexValue) {
    if(hexValue <= 0) {
        WARNING_MSG("command %s  - invalid call. The argument should be positive.\n", "test");
        return 2;
    }
    fprintf(stdout, "CMD TEST RESULT 0x%x\n", hexValue + 1);

    return CMD_OK_RETURN_VALUE;
}


/**
 * version de la commande test qui analyse la chaîne entrée à
 * l'interpreteur. Si la commande est correcte elle est executée.
 * Si la commande contient plusieurs parametres valides, elle
 * est excutée plusieurs fois.
 * @param inter l'interpreteur qui demande l'analyse
 * @return 0 en case de succes, un nombre positif sinon
 */

int testcmd(interpreteur inter) {
    DEBUG_MSG("Chaine : %s", inter->input);
    int return_value=0;

    int no_args=1;
    /* la commande test attend un argument hexa */
    int hexValue;
    char * token=NULL;

    /* la boucle permet d'executer la commande de manière recurrente*/
    /* i.e., tant qu'il y a un hexa on continue*/
    while((token = get_next_token(inter))!=NULL && return_value==0) {
        no_args=0;
        switch(get_type(token)) {
        case HEXA:
            sscanf(token,"%x",&hexValue);
            return_value = _testcmd(hexValue);
            break;
        default :
            WARNING_MSG("value %s is not a valid argument of command %s\n",token,"testcmd");
            return 1;
        }
    }


    if(no_args) {
        WARNING_MSG("no argument given to command %s\n","testcmd");
        return 1;
    }


    return return_value;
}


/**
 * commande exit qui ne necessite pas d'analyse syntaxique
 * @param inter l'interpreteur qui demande l'analyse
 * @return 0 en case de succes, un nombre positif sinon
 */
int exitcmd(interpreteur inter) {
    INFO_MSG("Bye bye !");
    return CMD_EXIT_RETURN_VALUE;
}


/*************************************************************\
 Les deux fonctions principales de l'émulateur.
	execute_cmd: parse la commande et l'execute en appelant la bonne fonction C
	acquire_line : recupere une ligne (donc une "commande") dans le flux
 \*************************************************************/


/**
*
* @brief parse la chaine courante de l'interpreteur à la recherche d'une commande, et execute cette commande.
* @param inter l'interpreteur qui demande l'analyse
* @return CMD_OK_RETURN_VALUE si la commande s'est exécutée avec succès (0)
* @return CMD_EXIT_RETURN_VALUE si c'est la commande exit. Dans ce cas, le programme doit se terminer. (-1)
* @return CMD_UNKOWN_RETURN_VALUE si la commande n'est pas reconnue. (-2)
* @return tout autre nombre (eg tout nombre positif) si erreur d'execution de la commande
*/
int execute_cmd(interpreteur inter, registre r,mem *memory,bp * bpa) {
    DEBUG_MSG("input '%s'", inter->input);
    char cmdStr[MAX_STR];

    memset( cmdStr, '\0', MAX_STR );
    int b=1;





    /* gestion des commandes vides, commentaires, etc*/
    if(strlen(inter->input) == 0
            || sscanf(inter->input, "%s", cmdStr) == 0
            || strlen(cmdStr) == 0
            || cmdStr[0] == '#') { /* ligne commence par # => commentaire*/
        return CMD_OK_RETURN_VALUE;
    }

    /*on identifie la commande avec un premier appel à get_next_token*/
    char * token = get_next_token(inter);

    if(strcmp(token, "exit") == 0) {
        return exitcmd(inter);
    }
    else if(strcmp(token, "test") == 0) {
        return testcmd(inter);
    }
    else if(strcmp(token, "test2") == 0) { //fonction pour essayer des test rapidement (fichiers tests marchent pas sur mac)
        
        int i=0;
        int n=(*memory)->nseg;
        for(i=0;i<n;i++){
            printf("nom segment %s \n",(*memory)->seg[i].name );
        }


        // int i=0; //vaddr32 c=0;
        /*char x[20];
        strcpy( x, "$pc" );
       printf("is_in_text %d %d %d\n",IsInText(*memory,0x2090),IsInText(*memory,0x3003), IsInText(*memory,0x5000) );

        printf("%d\n",convert(x));
*/

        //  printf("memory %p\n", *memory );
        // printf("nombre de segments %u\n", (*memory)->nseg );
        // for(i=0;i<(int)(*memory)->nseg;i++){

        //printf("adresse segment 0x %"PRIu32" +%d \n",(*memory)->seg->start,t*i); // *memory)->seg->start est un unint32
        //  c=(int)(*memory)->seg->start._64;
        //printf("%08x\n",c );


        //printf(" %"PRIu32"\n",c );
        //printf("adresse segment 0x%.8s  \n",itoa(c,16));
        //  }
        //int a=350;


        //setRegisterValue(r,31,a);
        //printf("%d\n", getRegisterValueByStr(r, "$31"));
        // printf("%s\n",itoa(a,16) );
        // printf("%d\n",is_hexa("0x33AB") );
    }
    else if(strcmp(token, "disp") == 0) {
        return dispcmd(inter,r,*memory);
    }

    else if(strcmp(token, "set") == 0) {
        return setcmd(inter,r,*memory);
    }

    else if(strcmp(token, "load") == 0) {
        return loadcmd(inter,memory);
    }

    else if(strcmp(token, "assert") == 0) {
        return assertcmd(inter,r, *memory);
    }

    else if(strcmp(token, "debug") == 0) {
        debugcmd(inter);
        return CMD_OK_RETURN_VALUE;
    }

    else if(strcmp(token, "resume") == 0) {
        resumecmd(inter);
        return CMD_OK_RETURN_VALUE;
    }

    else if (strcmp(token,"disasm")==0) {
        return disasmcmd(inter,*memory);
    }
    else if (strcmp(token,"step")==0) {
        return step(inter,r,*memory,&b);
    }
     else if (strcmp(token,"run")==0) {
         run(inter,r,*memory,*bpa);
         return CMD_OK_RETURN_VALUE;
    }
    else if (strcmp(token,"break")==0) {
        breakcmd(inter,*memory, bpa);
       // DEBUG_MSG("bpa apres break dans emulMips.c %p",*bpa); 
        return CMD_OK_RETURN_VALUE;
    }
else {
    WARNING_MSG("Unknown Command : '%s'\n", cmdStr);
    return CMD_UNKOWN_RETURN_VALUE;
}
}





/**
 * @param in Input line (possibly very badly written).
 * @param out Line in a suitable form for further analysis.
 * @return nothing
 * @brief This function will prepare a line for further analysis.
 *
 * This function will prepare a line for further analysis and check for low-level syntax errors.
 * colon, brackets, commas are surrounded with blank; tabs are replaced with blanks.
 * negs '-' are attached to the following token (e.g., "toto -   56" -> "toto -56")  .
 */
void string_standardise( char* in, char* out ) {
    unsigned int i=0, j;

    for ( j= 0; i< strlen(in); i++ ) {

        /* insert blanks around special characters*/
        if (in[i]==':' || in[i]=='+' || in[i]=='~') {
            out[j++]=' ';
            out[j++]=in[i];
            out[j++]=' ';

        }

        /* remove blanks after negation*/
        else if (in[i]=='-') {
            out[j++]=' ';
            out[j++]=in[i];
            while (isblank((int) in[i+1])) i++;
        }

        /* insert one blank before comments */
        else if (in[i]=='#') {
            out[j++]=' ';
            out[j++]=in[i];
        }
        /* translate tabs into white spaces*/
        else if (isblank((int) in[i])) out[j++]=' ';
        else out[j++]=in[i];
    }
}


/**
 *
 * @brief extrait la prochaine ligne du flux fp.
 * Si fp ==stdin, utilise la librairie readline pour gestion d'historique de commande.
 *
 * @return 0 si succes.
 * @return un nombre non nul si aucune ligne lue
 */
int  acquire_line(FILE *fp, interpreteur inter) {
    char* chunk =NULL;

    memset(inter->input, '\0', MAX_STR );
    inter->first_token =0;
    if (inter->mode==SCRIPT) {
        // mode fichier
        // acquisition d'une ligne dans le fichier
        chunk =calloc(MAX_STR, sizeof(*chunk));
        char * ret = fgets(chunk, MAX_STR, fp );
        if(ret == NULL) {
            free(chunk);
            return 1;
        }
        // si windows remplace le \r du '\r\n' (fin de ligne windows) par \0
        if(strlen(ret) >1 && ret[strlen(ret) -2] == '\r') {
            ret[strlen(ret)-2] = '\0';
        }
        // si unix remplace le \n par \0
        else if(strlen(ret) >0 && ret[strlen(ret) -1] == '\n') {
            ret[strlen(ret)-1] = '\0';
        }

    }
    else {
        /* mode shell interactif */
        /* on utilise la librarie libreadline pour disposer d'un historique */
        chunk = readline( PROMPT_STRING );
        if (chunk == NULL || strlen(chunk) == 0) {
            /* commande vide... */
            if (chunk != NULL) free(chunk);
            return 1;
        }
        /* ajout de la commande a l'historique, librairie readline */
        add_history( chunk );

    }
    // standardisation de la ligne d'entrée (on met des espaces là ou il faut)
    string_standardise(chunk, inter->input);

    free( chunk ); // liberation de la mémoire allouée par la fonction readline() ou par calloc()

    DEBUG_MSG("Ligne acquise '%s'\n", inter->input); /* macro DEBUG_MSG : uniquement si compil en mode DEBUG_MSG */
    return 0;
}


/****************/
void usage_ERROR_MSG( char *command ) {
    fprintf( stderr, "Usage: %s [file.emul]\n   If no file is given, executes in Shell mode.", command );
}


/**
 * Programme principal
 */
int main ( int argc, char *argv[] ) {
    /* exemples d'utilisation des macros du fichier notify.h */
    INFO_MSG("Debut du programme %s", argv[0]); /* macro INFO_MSG */
    interpreteur inter=init_inter(); /* structure gardant les infos et états de l'interpreteur*/
    FILE *fp = NULL; /* le flux dans lequel les commande seront lues : stdin (mode shell) ou un fichier */

    registre r=NULL;
    r = registre_new(35);
    mem memory = NULL;
    setRegisterValue(r,31,0xff7ff000);
 

    if ( argc > 2 ) {
        usage_ERROR_MSG( argv[0] );
        exit( EXIT_FAILURE );
    }
    if(argc == 2 && strcmp(argv[1], "-h") == 0) {
        usage_ERROR_MSG( argv[0] );
        exit( EXIT_SUCCESS );
    }

    /*par defaut : mode shell interactif */
    fp = stdin;
    inter->mode = INTERACTIF;
    if(argc == 2) {
        /* mode fichier de commandes */
        fp = fopen( argv[1], "r" );
        if ( fp == NULL ) {
            perror( "fopen" );
            exit( EXIT_FAILURE );
        }
        inter->mode = SCRIPT;
    }

    
     bp bpi=NULL;
 

    /* boucle infinie : lit puis execute une cmd en boucle */
    while ( 1 ) {



        if (acquire_line( fp,  inter)  == 0 ) {
            /* Une nouvelle ligne a ete acquise dans le flux fp*/

            int res = execute_cmd(inter,r,&memory,&bpi); /* execution de la commande */

            // traitement des erreurs
            switch(res) {
            case CMD_OK_RETURN_VALUE:
                break;
            case CMD_EXIT_RETURN_VALUE:
                /* sortie propre du programme */
                if ( fp != stdin ) {
                    fclose( fp );
                }
                del_inter(inter);
                registre_del(r);
                del_mem(memory);
                exit(EXIT_SUCCESS);
                break;
            default:
                /* erreur durant l'execution de la commande */
                /* En mode "fichier" toute erreur implique la fin du programme ! */
                if (inter->mode == SCRIPT) {
                    fclose( fp );
                    del_inter(inter);
                    registre_del(r);
                    del_mem(memory);
                    /*macro ERROR_MSG : message d'erreur puis fin de programme ! */
                    ERROR_MSG("ERREUR DETECTEE. Aborts");
                }
                break;
            }
        }
        if( inter->mode == SCRIPT && feof(fp) ) {
            /* mode fichier, fin de fichier => sortie propre du programme */
            DEBUG_MSG("FIN DE FICHIER");
            fclose( fp );
            del_inter(inter);
            registre_del(r);
            del_mem(memory);
            exit(EXIT_SUCCESS);
        }
    }
    /* tous les cas de sortie du programme sont gérés plus haut*/
    ERROR_MSG("SHOULD NEVER BE HERE\n");
}
