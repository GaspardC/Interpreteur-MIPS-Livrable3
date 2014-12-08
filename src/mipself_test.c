/*
 * \author (m) François Portet 2013 <francois.portet@imag.fr>
 * \brief test main for loading ELF file in memory.
 *--------------------------------------------------------------------------
 * Ce programme est un exemple de comment charger un fichier ELF
 * dans une mémoire virtuelle en utilisant l'API elf.
 *
 * ATTENTION, il s'agit juste d'un exemple et ne peut pas être utilisé tel quel
 * dans votre code!
 * A vous de vous en inspirer
 *
 */


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include "common/bits.h"
#include "common/notify.h"
#include "elf/elf.h"
#include "elf/syms.h"
#include "mem.h"
#include "elf/relocator.h"
#include "asm.h"
#include "lib.h"

// On fixe ici une adresse basse dans la mémoire virtuelle. Le premier segment
// ira se loger à cette adresse.
#define START_MEM 0x3000
// nombre max de sections que l'on extraira du fichier ELF
#define NB_SECTIONS 4

// nom de chaque section
#define TEXT_SECTION_STR ".text"
#define RODATA_SECTION_STR ".rodata"
#define DATA_SECTION_STR ".data"
#define BSS_SECTION_STR ".bss"

//nom du prefix à appliquer pour la section
#define RELOC_PREFIX_STR ".rel"

// Fonction permettant de verifier si une chaine de caracteres
// est bien dans la liste des symboles du fichier ELF
// parametres :
// 		name : le nom de la chaine recherchée
//  symtab : la table des symboles
//
// retourne 1 si present, 0 sinon
int is_in_symbols(char* name, stab symtab) {
    int i;
    for (i=0; i<symtab.size; i++) {
        if (!strcmp(symtab.sym[i].name,name)) return 1;
    }
    return 0;
}

// Cette fonction calcule le nombre de segments à prevoir
// Elle cherche dans les symboles si les sections predefinies
// s'y trouve
// parametres :
//  symtab : la table des symboles
//
// retourne le nombre de sections trouvées

unsigned int get_nsegments(stab symtab,char* section_names[],int nb_sections) {
    unsigned int n=0;
    int i;
    for (i=0; i<nb_sections; i++) {
        if (is_in_symbols(section_names[i],symtab)) n++;
    }
    return n;
}


// fonction permettant d'extraire une section du fichier ELF et de la chargée dans le segment du même nom
// parametres :
//   fp         : le pointeur du fichier ELF
//   memory     : la structure de mémoire virtuelle
//   scn        : le nom de la section à charger
//   permission : l'entier représentant les droits de lecture/ecriture/execution
//   add_start  : l'addresse virtuelle à laquelle la section doit être chargée
//
// retourne 0 en cas de succes, une valeur non nulle sinon
int elf_load_section_in_memory(FILE* fp, mem memory, char* scn,unsigned int permissions,unsigned long long add_start) {
    byte *ehdr    = __elf_get_ehdr( fp );
    byte *content = NULL;
    uint  textsz  = 0;
    vsize sz;
    vaddr addr;

    byte *useless = elf_extract_section_header_table( ehdr, fp );
    free( useless );

    if ( NULL == ehdr ) {
        WARNING_MSG( "Can't read ELF file" );
        return 1;
    }

    if ( 1 == attach_scn_to_mem(memory, scn, SCN_ATTR( WIDTH_FROM_EHDR( ehdr ), permissions ) ) ) {
        WARNING_MSG( "Unable to create %s section", scn );
        free( ehdr );
        return 1;
    }

    content = elf_extract_scn_by_name( ehdr, fp, scn, &textsz, NULL );
    if ( NULL == content ) {
        WARNING_MSG( "Corrupted ELF file" );
        free( ehdr );
        return 1;
    }

    switch( WIDTH_FROM_EHDR(ehdr) ) {
    case 32 :
        sz._32   = textsz/*+8*/; /* +8: In case adding a final sys_exit is needed */
        addr._32 = add_start;
        break;
    case 64 :
        sz._64   = textsz/*+8*/; /* +8: In case adding a final sys_exit is needed */
        addr._64 = add_start;
        break;
    default :
        WARNING_MSG( "Wrong machine width" );
        return 1;
    }

    if ( 1 == fill_mem_scn(memory, scn, sz, addr, content ) ) {
        free( ehdr );
        free( content );
        WARNING_MSG( "Unable to fill in %s segment", scn );
        return 1;
    }

    free( content );
    free( ehdr );

    return 0;
}

/*--------------------------------------------------------------------------  */
/**
 * @param fp le fichier elf original
 * @param seg le segment à reloger
 * @param mem l'ensemble des segments
 *
 * @brief Cette fonction effectue la relocation du segment passé en parametres
 * @brief l'ensemble des segments doit déjà avoir été chargé en memoire.
 *
 * VOUS DEVEZ COMPLETER CETTE FONCTION POUR METTRE EN OEUVRE LA RELOCATION !!
 */
void reloc_segment(FILE* fp, segment seg, mem memory,unsigned int endianness,stab symtab) {
    byte *ehdr    = __elf_get_ehdr( fp );
    uint32_t  scnsz  = 0;
    Elf32_Rel *rel = NULL;
    char* reloc_name = malloc(strlen(seg.name)+strlen(RELOC_PREFIX_STR)+1);
    scntab section_tab;

    // on recompose le nom de la section
    memcpy(reloc_name,RELOC_PREFIX_STR,strlen(RELOC_PREFIX_STR)+1);
    strcat(reloc_name,seg.name);
    
    // on récupere le tableau de relocation et la table des sections
    rel = (Elf32_Rel *)elf_extract_scn_by_name( ehdr, fp, reloc_name, &scnsz, NULL );
    elf_load_scntab(fp ,32, &section_tab);



    if (rel != NULL &&seg.content!=NULL && seg.size._32!=0) {

        INFO_MSG("--------------Relocation de %s-------------------",seg.name) ;
        INFO_MSG("Nombre de symboles a reloger: %ld\n",scnsz/sizeof(*rel)) ;


        //------------------------------------------------------

        //TODO : faire la relocation ICI !
        int i;
        for(i=0;i<scnsz/sizeof(*rel);i++)
        {
            FLIP_ENDIANNESS(rel[i].r_offset);
            uint32_t offset = rel[i].r_offset;
            FLIP_ENDIANNESS(rel[i].r_info);
            uint32_t sym = ELF32_R_SYM(rel[i].r_info);
            unsigned char type= ELF32_R_TYPE(rel[i].r_info);
            INFO_MSG("offset :%x | sym:%x -> %s | type:%x -> %s", offset, sym, symtab.sym[sym].name, type, MIPS32_REL[type]);
            
            //stab32_print( symtab );
            
            //extraction du mot
            uint32_t addr = seg.start._32 + offset;
            uint32_t word = loadmem(addr,memory,"WORD");            
            uint32_t V,A,S,P;
            
            switch(type)
            {
            	case R_MIPS_32 :
            	
            	    A=word;
            	    S=memory->seg[sym-1].start._32;
            	    V=S+A;
            	    INFO_MSG("P (adresse) : %x  A : %x  S: %x -> V : %x",addr,A,S,V);
            	    word=V;
            	    
            	break;
            	
            	case R_MIPS_26 :
            	
            	    A=getbits(word,0,25);
            	    P=addr;
            	              	    
            	    V=((A<<2)|((P & 0xF0000000)+S))>>2; // cas STT_SECTION  
            	    INFO_MSG("P (adresse) : %x  A : %x  S: %x -> V : %x",addr,A,S,V);    	     
            	    word=((word&0xfc000000))|(V&0x03ffffff);
            	    
            	break;
            	
            	case R_MIPS_HI16 :
            	
            	break;
            	
            	case R_MIPS_LO16 :
            	
            	break;
            	
            	default :
            	
            	break;
            }
            
            storemem(addr,word,memory,"WORD");
        }

        //------------------------------------------------------

    }
    del_scntab(section_tab);
    free( rel );
    free( reloc_name );
    free( ehdr );

}


// fonction affichant les octets d'un segment sur la sortie standard
// parametres :
//   seg        : le segment de la mémoire virtuelle à afficher

void print_segment_raw_content(segment* seg) {
    int k;
    int word =0;
    if (seg!=NULL && seg->size._32>0) {
        for(k=0; k<seg->size._32; k+=4) {
            if(k%16==0) printf("\n  0x%08x ",k);
            word = *((unsigned int *) (seg->content+k));
            FLIP_ENDIANNESS(word);
            printf("%08x ",	word);
        }
    }
}

