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

#include <stdint.h>


/********************************************\
		Commande load
\********************************************/
int
loadcmd (interpreteur inter, mem * memory)
{
    char *section_names[NB_SECTIONS] =
    {   TEXT_SECTION_STR, RODATA_SECTION_STR, DATA_SECTION_STR,
        BSS_SECTION_STR
    };
    unsigned int segment_permissions[NB_SECTIONS] = { R_X, R__, RW_, RW_ };
    unsigned int nsegments;
    int i = 0, j = 0;
    unsigned int type_machine;
    unsigned int endianness;	//little ou big endian
    unsigned int bus_width;	// 32 bits ou 64bits
    unsigned int next_segment_start = START_MEM;	// compteur pour designer le début de la prochaine section
    char *chaine = NULL;

    if ((chaine = get_next_token (inter)) != NULL)
    {

        //mem memory;
        stab symtab = new_stab (0);	// table des symboles
        FILE *pf_elf;

        if ((pf_elf = fopen (chaine, "r")) == NULL)
        {
            INFO_MSG ("cannot open file %s", chaine);
            return CMD_UNKOWN_RETURN_VALUE;
        }

        if (!assert_elf_file (pf_elf))
        {
            ERROR_MSG ("file %s is not an ELF file", chaine);
            return CMD_UNKOWN_RETURN_VALUE;
        }
        // recuperation des info de l'architecture
        elf_get_arch_info (pf_elf, &type_machine, &endianness, &bus_width);
        // et des symboles
        elf_load_symtab (pf_elf, bus_width, endianness, &symtab);


        nsegments = get_nsegments (symtab, section_names, NB_SECTIONS);

        // allouer la memoire virtuelle
        *memory = init_mem (nsegments);

        // Ne pas oublier d'allouer les differentes sections
        j = 0;
        for (i = 0; i < NB_SECTIONS; i++)
        {
            if (is_in_symbols (section_names[i], symtab))
            {
                elf_load_section_in_memory (pf_elf, *memory, section_names[i],
                                            segment_permissions[i],
                                            next_segment_start);
                next_segment_start += (((*memory)->seg[j].size._32 + 0x1000) >> 12) << 12;	// on arrondit au 1k suppérieur
                // print_segment_raw_content(&(*memory)->seg[j]);
                j++;
            }
        }

        del_stab (symtab);
        fclose (pf_elf);

        INFO_MSG ("elf file load with success");
        return CMD_OK_RETURN_VALUE;
    }
    INFO_MSG ("elf file required");
    return CMD_REQUIRE_ARG;

}


/********************************************\
		Commande disp
\********************************************/
int
dispcmd (interpreteur inter, registre r, mem memory)
{
    char *token = NULL;

    if (memory == NULL)
    {
        WARNING_MSG("elf file required !!!\n");
        return NO_ELF_LOAD;
    }

    if ((token = get_next_token (inter)) != NULL)
    {

        /*** MEMOIRE ***/
        if(strcmp(token, "mem") == 0) 
        {
            int c = 0;
            while ((token = get_next_token (inter)) != NULL)
            {

                if (strcmp (token, "map") == 0 && c == 0)
                {  
                    // printf ("\n------ Fichier ELF \"boucle.o\" : sections lues lors du chargement ------\n");
                    printf ("Virtual memory map (%d segments)",
                            memory->nseg);
                    print_mem (memory);
                    return CMD_OK_RETURN_VALUE;
                }


                switch (get_type (token))
                { 
                    int a = 0;
                    int b = 0;
                    char *saveA;
                    char *saveB;
                    int j, saveAI, saveBI;  
                                        				
                case HEXA:
                
		    
                    saveA = token;
                    //numero du segment
                    a = numero_segment (token, memory);
                    
                    if (a == -1)
                    {
                        WARNING_MSG ("memoire virtuelle indisponible !!!");
                        return CMD_WRONG_ARG;
                    }
                    
                    token = get_next_token (inter);
                    if (token == NULL)
                    {
                        WARNING_MSG ("Utiliser la syntaxe 0x0000:0x0000");
                        return CMD_WRONG_ARG;
                    }

                    if (strcmp (token, ":") != 0)
                    {
                        WARNING_MSG ("Mettre des deux points entre les valeurs des adresses en hexa");
                        return CMD_WRONG_ARG;
                    }
                    
                    token = get_next_token (inter);                    
                    if (token == NULL)
                    {
                        WARNING_MSG ("Utiliser la syntaxe 0x0000:0x0000");
                        return CMD_WRONG_ARG;
                    }
                    
                    if(get_type (token)==HEXA)
                    {
                        saveB = token;
                        //numero du segment
                        b = numero_segment (token, memory);
                        if (b == -1)
                        {
                            WARNING_MSG ("memoire virtuelle indisponible !!!");
                            return CMD_WRONG_ARG;
                        }
                        
                        if (b != a)
                        {
                            WARNING_MSG ("Segment differents !!!");
                            return CMD_WRONG_ARG;
                        }
                        
                        sscanf (saveA, "%x", &saveAI);	
                        sscanf (saveB, "%x", &saveBI);
                        
                        if (saveAI > saveBI)
                        {
                            WARNING_MSG ("Plage d'adresse non conforme !!!");
                            return CMD_WRONG_ARG;
                        }
                        
                        for (j = saveAI; j <= saveBI; j++)
                        {

                            if (j % 16 == 0)
                            printf ("\n0x%08x  ", j);
                            printf ("%02x ", *(memory->seg[a].content + (j - memory->seg[a].start._64)));
                        }
                    }

		    else 
		    {
                        WARNING_MSG("value \"%s\" is not a valid argument of command %s", token, "disp");
                        return 1;
                    }

                    break;
                default:
                    WARNING_MSG("value %s is not a valid argument of command %s",token, "disp");
                    return CMD_WRONG_ARG;
                }
            }

        }

        /*** REGISTRE ***/
        else if (strcmp (token, "reg") == 0)
        {
            int c = 0;

            while ((token = get_next_token (inter)) != NULL)
            {
                if (strcmp (token, "all") == 0 && c == 0)
                {
                    int i;
                    for (i = 0; i < 35; i++)
                    {
                        c++;
                        printf ("%s : %.8x\t", getName2 (i),
                                getRegisterValue (r, i));
                        if (c % 4 == 0)
                            printf ("\n");
                    }
                    printf ("\n");
                    return CMD_OK_RETURN_VALUE;
                }

                switch (get_type (token))
                {

                case REGISTRE:

                    c++;
                    printf ("%s : %.8x\t", getName (token),
                            getRegisterValueByStr (r, token));
                    if (c % 4 == 0)
                        printf ("\n");
                    break;

                default:
                    WARNING_MSG("value %s is not a valid argument of command %s",token, "disp");
                    return CMD_UNKOWN_RETURN_VALUE;
                    break;
                }
            }
        }
        printf ("\n");
        return CMD_OK_RETURN_VALUE;
    }
    return CMD_REQUIRE_ARG;
}


/********************************************\
		Commande set
\********************************************/
int
setcmd (interpreteur inter, registre r, mem memory)
{ 


    char *token = NULL;
    char *a = NULL;
    char *b = NULL;

    if (memory == NULL)
    {
        WARNING_MSG("elf file required !!!");
        return CMD_WRONG_ARG;
    }

    token = get_next_token (inter);
    if (token == NULL)
    {
        WARNING_MSG("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur> \n\"set\" \"reg\" <registre> <valeur>");
        return CMD_WRONG_ARG;
    }

    if (strcmp (token, "reg") != 0 && strcmp (token, "mem") != 0)
    {
        WARNING_MSG ("Structure valide :\n\"set\" \"reg\" <registre> <valeur>");
        return CMD_WRONG_ARG;
    }


    if (strcmp (token, "reg") == 0)
    {

        token = get_next_token (inter);
        if (token == NULL)
        {
            WARNING_MSG ("Structure valide :\n\"set\" \"reg\" <registre> <valeur>");
            return CMD_WRONG_ARG;
        }

        switch (get_type (token))
        {

        case REGISTRE:

            a = strdup (token);
            token = get_next_token (inter);
            if (token == NULL)
            {
                WARNING_MSG ("Structure valide :\n\"set\" \"reg\" <registre> <valeur>");
                return CMD_WRONG_ARG;
            }

            b = strdup (token);
            setRegisterValue (r, convert (a), atoi (b));
            return CMD_OK_RETURN_VALUE;
            break;

        default:
            WARNING_MSG ("value %s is not a valid argument of command %s", token, "set");
            return CMD_WRONG_ARG;
            break;
        }
    }


    if (strcmp (token, "mem") == 0)
    {

        token = get_next_token (inter);

        if ((token) == NULL)
        {
            WARNING_MSG ("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
            return CMD_WRONG_ARG;
        }

        char *aa;
        int n = 0;
        int bI;
        int v;

        aa = strdup (token);	// byte ou word
        
        
        if ((strcmp (aa, "byte") != 0) && (strcmp (aa, "word") != 0))
        {
            WARNING_MSG ("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
            return CMD_WRONG_ARG;
        }
        
	/*** byte ***/
        if (strcmp (aa, "byte") == 0)
        {

            token = get_next_token (inter);
            if ((token) == NULL)
            {
                WARNING_MSG ("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
                return CMD_WRONG_ARG;
            }
            
            //printf("token : %s\n",token );
            if (get_type (token) != HEXA)
            {
                WARNING_MSG ("adresse doit être donnée en hexa");
                return CMD_WRONG_ARG;
            }

            sscanf (token, "%x", &bI);

            n = numero_segment (token, memory);
            if (n == -1)
            {
                WARNING_MSG ("memoire virtuelle indisponible !!!");
                return CMD_WRONG_ARG;
            }
            
            token = get_next_token (inter);

            if ((token) == NULL)
            {
                WARNING_MSG ("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
                return CMD_WRONG_ARG;
            }


            sscanf (token, "%x", &v);

            if ((int) v > 256)
            {
                WARNING_MSG ("your writed 'byte' but the value is a word");
                return CMD_WRONG_ARG;
            }

            *(memory->seg[n].content + (bI - memory->seg[n].start._64)) = v;
            return CMD_OK_RETURN_VALUE;
        }
	
	/*** word ***/
        if (strcmp (aa, "word") == 0)
        {
            token = get_next_token (inter);

            if (token == NULL)
            {
                WARNING_MSG ("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
          	return CMD_WRONG_ARG;
            }

            if (get_type (token) != HEXA)
            {
                WARNING_MSG ("adresse doit être donnée en hexa");
                return CMD_WRONG_ARG;
            }

            sscanf (token, "%x", &bI);

            n = numero_segment (token, memory);
            if (n == -1)
            {
                WARNING_MSG ("memoire virtuelle indisponible !!!");
                return CMD_WRONG_ARG;
            }

            token = get_next_token (inter);

            if (token == NULL)
            {
                WARNING_MSG ("Structure valide :\n\"set\" \"mem\" <type> <adresse> <valeur>");
          	return CMD_WRONG_ARG;
            }

            sscanf (token, "%x", &v);
            DEBUG_MSG ("value %08x\n", v);

            uint32_t a = v & 0x000000FF;
            uint32_t b = (v & 0x0000FF00) >> 8;
            uint32_t c = (v & 0x00FF0000) >> 16;
            uint32_t d = (v & 0xFF000000) >> 24;

            *(memory->seg[n].content + (bI + 3 - memory->seg[n].start._64)) = a;
            *(memory->seg[n].content + (bI + 2 - memory->seg[n].start._64)) = b;
            *(memory->seg[n].content + (bI + 1 - memory->seg[n].start._64)) = c;
            *(memory->seg[n].content + (bI - memory->seg[n].start._64)) = d;

            return CMD_OK_RETURN_VALUE;
        }
    }

    INFO_MSG ("None");
    return CMD_UNKOWN_RETURN_VALUE;
}



/********************************************\
		Commande assert
\********************************************/
int
assertcmd (interpreteur inter, registre r, mem memory)
{
    char *token = NULL;
    int a = 0;
    token = get_next_token (inter);
    int bI;			// adresse hexa
    int v;			// valeur
    int n;

    if (memory == NULL)
    {
        printf ("load an elf file before\n");
        return CMD_WRONG_ARG;
    }


    if (token == NULL)
    {
        WARNING_MSG ("argument required\n");
        return CMD_REQUIRE_ARG;
    }

    if (strcmp (token, "reg") == 0)
    {
        if ((token = get_next_token (inter)) != NULL)
        {
            switch (get_type (token))
            {

            case REGISTRE:
                a = getRegisterValue (r, convert (token));
                if ((token = get_next_token (inter)) != NULL)
                {

                    if (a == atoi (token))
                    {   //***********-------> PROBLEME !!! Marche pour "assert reg $1 dhcsjd"
                        INFO_MSG ("Valeurs identiques\n");
                        return 0;
                    }
                    else
                    {
                        INFO_MSG ("Valeurs differentes\n");
                        return 1;
                    }
                }
                else
                {
                    WARNING_MSG ("argument required\n");
                    return CMD_REQUIRE_ARG;
                }
                break;

            default:
                WARNING_MSG ("register type required\n");
                return CMD_WRONG_ARG;
                break;
            }
        }
        else
        {
            WARNING_MSG ("argument required\n");
            return CMD_REQUIRE_ARG;
        }

    }

    if (strcmp (token, "byte") == 0)
    {

        token = get_next_token (inter);

        if (token == NULL)
            return CMD_WRONG_ARG;
        printf ("token : %s\n", token);
        if (get_type (token) != HEXA)
        {
            return CMD_WRONG_ARG;
            DEBUG_MSG ("pas hexa");
        }

        sscanf (token, "%x", &bI);	//adresse en hexa

        n = numero_segment (token, memory);
        token = get_next_token (inter);

        if ((token) == NULL)
        {
            DEBUG_MSG ("pb");
            return CMD_WRONG_ARG;
        }


        sscanf (token, "%x", &v);

        if ((int) v > 256)
        {
            INFO_MSG ("your writed 'byte' but the value is a word");
            return CMD_WRONG_ARG;
        }


        if (v == *(memory->seg[n].content + (bI - memory->seg[n].start._64)))
        {
            INFO_MSG ("Ok");
            return CMD_OK_RETURN_VALUE;
        }
        else
        {
            DEBUG_MSG ("pas egaux");
            return CMD_WRONG_ARG;
        }

    }

    if (strcmp (token, "word") == 0)
    {

        token = get_next_token (inter);

        if (token == NULL)
            return CMD_WRONG_ARG;
        printf ("token : %s\n", token);
        if (get_type (token) != HEXA)
        {
            return CMD_WRONG_ARG;
            DEBUG_MSG ("pas hexa");
        }

        sscanf (token, "%x", &bI);	//adresse en hexa

        n = numero_segment (token, memory);
        token = get_next_token (inter);

        if ((token) == NULL)
        {
            DEBUG_MSG ("pb");
            return CMD_WRONG_ARG;
        }


        sscanf (token, "%x", &v);

        uint32_t combined =
            (*(memory->seg[n].content + (bI - memory->seg[n].start._64))) << 24 |
            (*(memory->seg[n].content + (bI + 1 - memory->seg[n].start._64))) <<
            16 | (*(memory->seg[n].content + (bI + 2 - memory->seg[n].start._64)))
            << 8 |
            (*(memory->seg[n].content + (bI + 3 - memory->seg[n].start._64)));
        printf ("%x\n", combined);
        if (v == combined)
        {
            INFO_MSG ("Ok");
            return CMD_OK_RETURN_VALUE;
        }
        else
        {
            DEBUG_MSG ("pas egaux");
            return CMD_WRONG_ARG;
        }
    }

    WARNING_MSG ("argument required\n");
    return CMD_REQUIRE_ARG;
}

/********************************************\
		Commande debug
\********************************************/

void
debugcmd (interpreteur inter)
{
    inter->mode = INTERACTIF;
}

/********************************************\
		Commande resume
\********************************************/

void
resumecmd (interpreteur inter)
{
    inter->mode = SCRIPT;
}

/********************************************\
		Commande disasm
\********************************************/

int
disasmcmd (interpreteur inter, mem memory)
{

    if (memory == NULL)
    {
        WARNING_MSG("elf file required");
        return CMD_WRONG_ARG;
    }

    char *token = NULL;
    char *addA = NULL;
    char *addB = NULL;
    char *inc = NULL;
    int addAI, addBI;
    int incI;
    int n, m;
    int b;
    int i = 0;
    int combined;

    if ((token = get_next_token (inter)) == NULL)
    {
        WARNING_MSG ("Structure valide :\n \"disasm\" <plage>+");
        return CMD_WRONG_ARG;
    }

    if (get_type (token) != HEXA)
    {
        WARNING_MSG ("not hexa");
        return CMD_WRONG_ARG;
    }

    addA = token;

    if ((token = get_next_token (inter)) == NULL)
    {
        WARNING_MSG ("Structure valide :\n \"disasm\" <plage>+");
        return CMD_WRONG_ARG;
    }

    switch (get_type (token))
    {
    case PLUS:
        if ((token = get_next_token (inter)) == NULL)
        {
            WARNING_MSG ("Valeur requise");
            return CMD_WRONG_ARG;
        }
        inc = token;
        sscanf (addA, "%x", &addAI);
        sscanf (inc, "%x", &incI);
        incI = incI - (incI % 4);
        n = numero_segment (addA, memory);
        if (n == -1)
        {
            INFO_MSG ("memory out of bonds");
            return CMD_WRONG_ARG;
        }
        sscanf (addAI + inc, "%x", &b);
        /*
           char s[100];
           strcat(s,"0x");
           strcat(s,(char*)b);

           if(numero_segment(s,memory)!=0)
           {DEBUG_MSG("out of .text");
           return CMD_WRONG_ARG;
           }
         */


        for (i = addAI; i < addAI + incI; i++)
        {
            if (i % 4 == 0)
            {

                combined = (*(memory->seg[n].content + (i - memory->seg[n].start._64))) << 24 |(*(memory->seg[n].content +(i + 1 -memory->seg[n].start._64))) << 16 | (*(memory->seg[n].content + (i + 2 -memory->seg[n].start._64))) <<8 |(*(memory->seg[n].content + (i + 3 - memory->seg[n].start._64)));
                
                printf ("%x :: %.8x\t", i, combined);
                dispAssem (combined);
            }
        }
        return CMD_OK_RETURN_VALUE;
        break;

    case DEUXPOINTS:
        if ((token = get_next_token (inter)) == NULL)
        {
            DEBUG_MSG ("Erreur");
            return CMD_WRONG_ARG;
        }
        /*verifier que zone mémoire dans segment */
        addB = token;
        sscanf (addA, "%x", &addAI);
        sscanf (addB, "%x", &addBI);
        n = numero_segment (addA, memory);
        m = numero_segment (addB, memory);

        if (-1 == n)
        {
            WARNING_MSG ("Mauvaise adresse %s", addA);
            return CMD_WRONG_ARG;
        }

        if (-1 == m)
        {
            WARNING_MSG ("Mauvaise adresse %s", addB);
            return CMD_WRONG_ARG;
        }

        if (n != 0 || m != 0)
        {
            DEBUG_MSG ("adresse non dans .text");
            return CMD_WRONG_ARG;
        }

        for (i = addAI; i <= addBI; i++)
        {
            if (i % 4 == 0)
            {   //une instruction tous les 4 (=word de 32 bits)

                combined = (*(memory->seg[n].content + (i -memory->seg[n].start._64)))<< 24 |(*(memory->seg[n].content +(i + 1 -
memory->seg[n].start._64))) << 16 | (*(memory->seg[n].content + (i + 2 -memory->seg[n].start._64))) <<8 | (*(memory->seg[n].content +(i + 3 - memory->seg[n].start._64)));

                printf ("%x :: %.8x\t", i, combined);
                dispAssem (combined);
            }
        }
        return CMD_OK_RETURN_VALUE;
        break;

    default:
        WARNING_MSG ("register type required\n");
        return CMD_WRONG_ARG;
        break;
    }

    INFO_MSG ("ni : ni + : respercter la syntaxe");
    return CMD_WRONG_ARG;
}

/********************************************\
	Fonction numero de segment
\********************************************/

int numero_segment (char *chaine, mem memory)
{
    uint32_t a = 0, b = 0, c = 0;
    sscanf (chaine, "%x", &a);
    int i = 0;			//i =n numero de segment
    // printf("%x\n",a );

    for (i = 0; i < memory->nseg; i++)
    {
        b = (uint32_t) memory->seg[i].start._64;
        c = (uint32_t) memory->seg[i + 1].start._64;
        //printf("BORNES SEG%x %x\n",b,c );

        if (b <= a && a < c)
        {
            return i;
        }

    }
    return -1;
}
