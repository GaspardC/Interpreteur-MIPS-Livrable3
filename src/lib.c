#include "lib.h"

uint32_t getbits(uint32_t u, size_t start, size_t stop)
{
    size_t n=31-(stop-start);
    size_t m=31-stop;

    uint32_t tmp = 0xffffffff;
    tmp = tmp<<n; /*0xff000000*/
    tmp = tmp>>m; /*0x00ff0000*/

    return (u&tmp)>>start;

}

/**************************************\
    fonctions instructions assembleur
\**************************************/

uint32_t getRS(uint32_t u)
{
    return getbits(u,21,25);
}

uint32_t getRT(uint32_t u)
{
    return getbits(u,16,20);
}

uint32_t getRD(uint32_t u)
{
    return getbits(u,11,15);
}

int16_t getIm(uint32_t u)
{
    return getbits(u,0,15);
}

int16_t getOffset(uint32_t u)
{
    return getbits(u,0,15);
}

uint32_t getSA(uint32_t u)
{
    return getbits(u,6,10);
}

uint32_t getTarget(uint32_t u)
{
    return getbits(u,0,25);
}


char* getInstr(uint32_t u)
{
    uint32_t n=getbits(u,26,31);

    if(strcmp(type(n),"SPECIAL")==0) {
        uint32_t n=getbits(u,0,5);
        return libs(n);
    }

    if(strcmp(type(n),"REGIMM")==0) {
        uint32_t n=getbits(u,16,20);
        return libr(n);
    }

    if(strcmp(type(n),"SPECIAL3")==0) {
        uint32_t n=getbits(u,6,10);
        return libs3(n);
    }

    else return libn(n);
}

/**************************************\
	bibliothèque
\**************************************/

char* type(uint32_t u)
{
    if(u==0x0) {
        char* c = "SPECIAL";
        return c;
    }
    if(u==0x1) {
        char* c = "REGIMM";
        return c;
    }
    if(u==0x1f) {
        char* c = "SPECIAL3";
        return c;
    }
    else
    {
        char* c = "NORMAL";
        return c;
    }
}

char* libs(uint32_t u)
{
    if(u==0x20) {
        char* c = "ADD";
        return c;
    }
    if(u==0x21) {
        char* c = "ADDU";
        return c;
    }
    if(u==0x24) {
        char* c = "AND";
        return c;
    }
    if(u==0xd) {
        char* c = "BREAK";
        return c;
    }
    if(u==0x1a) {
        char* c = "DIV";
        return c;
    }
    if(u==0x9) {
        char* c = "JALR";
        return c;
    }
    if(u==0x8) {
        char* c = "JR";
        return c;
    }
    if(u==0x10) {
        char* c = "MFHI";
        return c;
    }
    if(u==0x12) {
        char* c = "MFLO";
        return c;
    }
    if(u==0x18) {
        char* c = "MULT";
        return c;
    }
    if(u==0x0) {
        char* c = "SLL";
        return c;
    }
    if(u==0x25) {
        char* c = "OR";
        return c;
    }
    if(u==0x2a) {
        char* c = "OR";
        return c;
    }
    if(u==0x2c) {
        char* c = "SLTU";
        return c;
    }
    if(u==0x3) {
        char* c = "SRA";
        return c;
    }
    if(u==0x2) {
        char* c = "SRL";
        return c;
    }
    if(u==0x22) {
        char* c = "SUB";
        return c;
    }
    if(u==0x23) {
        char* c = "SUBU";
        return c;
    }
    if(u==0xc) {
        char* c = "SYSCALL";
        return c;
    }
    if(u==0x26) {
        char* c = "XOR";
        return c;
    }
    else {
        char* c = "PROBLEME";
        DEBUG_MSG("Non connu %u",u);
        return c;
    }
}

char* libr(uint32_t u)
{
    if(u==0x1) {
        char* c = "BGEZ";
        return c;
    }
    if(u==0x0) {
        char* c = "BLTZ";
        return c;
    }
    else {
        char* c = "PROBLEME";
        DEBUG_MSG("Non connu %u",u);
        return c;
    }
}

char* libs3(uint32_t u)
{
    if(u==0x10) {
        char* c = "SEB";
        return c;
    }
    else {
        char* c = "PROBLEME";
        DEBUG_MSG("Non connu %u",u);
        return c;
    }
}
char* libn(uint32_t u)
{


    if(u==0x8) {
        char* c = "ADDI";
        return c;
    }
    if(u==0x9) {
        char* c = "ADDIU";
        return c;
    }
    if(u==0xc) {
        char* c = "ANDI";
        return c;
    }
    if(u==0x4) {
        char* c = "BEQ";
        return c;
    }
    if(u==0x7) {
        char* c = "BGTZ";
        return c;
    }
    if(u==0x6) {
        char* c = "BLEZ";
        return c;
    }
    if(u==0x5) {
        char* c = "BNE";
        return c;
    }
    if(u==0x2) {
        char* c = "J";
        return c;
    }
    if(u==0x3) {
        char* c = "JAL";
        return c;
    }
    if(u==0x9) {
        char* c = "JALR";
        return c;
    }
    if(u==0x20) {
        char* c = "LB";
        return c;
    }
    if(u==0x24) {
        char* c = "LBU";
        return c;
    }
    if(u==0xf) {
        char* c = "LUI";
        return c;
    }
    if(u==0x23) {
        char* c = "LW";
        return c;
    }
    if(u==0xd) {
        char* c = "ORI";
        return c;
    }
    if(u==0x28) {
        char* c = "SB";
        return c;
    }
    if(u==0xa) {
        char* c = "SLTI";
        return c;
    }
    if(u==0xb) {
        char* c = "SLTIU";
        return c;
    }
    if(u==0x2b) {
        char* c = "SW";
        return c;
    }
    else {
        char* c = "PROBLEME";
        DEBUG_MSG("Non connu %u",u);
        return c;
    }
}

/**************************************\
	Obtenir nom de registre
\**************************************/

char* getR(uint32_t a) {
    if(a==0x0) return "$zero";
    if(a==0x1) return "$at";
    if(a==0x2) return "$v0";
    if(a==0x3) return "$v1";
    if(a==0x4) return "$a0";
    if(a==0x5) return "$a1";
    if(a==0x6) return "$a2";
    if(a==0x7) return "$a3";
    if(a==0x8) return "$t0";
    if(a==0x9) return "$t1";
    if(a==0xa) return "$t2";
    if(a==0xb) return "$t3";
    if(a==0xc) return "$t4";
    if(a==0xd) return "$t5";
    if(a==0xe) return "$t6";
    if(a==0xf) return "$t7";
    if(a==0x10) return "$s0";
    if(a==0x11) return "$s1";
    if(a==0x12) return "$s2";
    if(a==0x13) return "$s3";
    if(a==0x14) return "$s4";
    if(a==0x15) return "$s5";
    if(a==0x16) return "$s6";
    if(a==0x17) return "$s7";
    if(a==0x18) return "$t8";
    if(a==0x19) return "$t9";
    if(a==0x1a) return "$k0";
    if(a==0x1b) return "$k1";
    if(a==0x1c) return "$gp";
    if(a==0x1d) return "$sp";
    if(a==0x1e) return "$fp";
    if(a==0x1f) return "$ra";
    if(a==0x20) return "$PC";
    if(a==0x21) return "$HI";
    if(a==0x22) return "$LO";

    else return "**registre non valide**";
}

/**************************************\
	affiche les commandes
\**************************************/

void dispAssem(uint32_t u)
{
    char * instr = getInstr(u);
    if(strcmp(instr,"ADD")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"ADDI")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRT(u)), getR(getRS(u)), getIm(u));
    }
    else if(strcmp(instr,"ADDIU")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRT(u)), getR(getRS(u)), getIm(u));
    }
    else if(strcmp(instr,"ADDU")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"AND")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"ANDI")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRT(u)), getR(getRS(u)), getIm(u));
    }
    else if(strcmp(instr,"BEQ")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRS(u)), getR(getRT(u)), getOffset(u)<<2);
    }
    else if(strcmp(instr,"BGEZ")==0) {
        printf("%s %s, %d\n",getInstr(u), getR(getRS(u)), getOffset(u)<<2);
    }
    else if(strcmp(instr,"BGTZ")==0) {
        printf("%s %s, %d\n",getInstr(u), getR(getRS(u)), getOffset(u)<<2);
    }
    else if(strcmp(instr,"BLEZ")==0) {
        printf("%s %s, %d\n",getInstr(u), getR(getRS(u)), getOffset(u)<<2);
    }
    else if(strcmp(instr,"BLTZ")==0) {
        printf("%s %s, %d\n",getInstr(u), getR(getRS(u)), getOffset(u)<<2);
    }
    else if(strcmp(instr,"BNE")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRS(u)), getR(getRT(u)), getOffset(u)<<2);
    }
    else if(strcmp(instr,"BREAK")==0) {
        printf("%s\n",getInstr(u));
    }
    else if(strcmp(instr,"DIV")==0) {
        printf("%s %s, %s\n",getInstr(u), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"J")==0) {
        printf("%s %d\n",getInstr(u), getTarget(u)<<2);
    }
    else if(strcmp(instr,"JAL")==0) {
        printf("%s %d\n",getInstr(u), getTarget(u)<<2);
    }
    else if(strcmp(instr,"JARL")==0) {
        printf("%s %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)));
    }
    else if(strcmp(instr,"JR")==0) {
        printf("%s %s\n",getInstr(u), getR(getRS(u)));
    }
    else if(strcmp(instr,"LB")==0) {
        printf("%s %s, %d (%s)\n",getInstr(u), getR(getRT(u)), getOffset(u), getR(getRS(u)));
    }
    else if(strcmp(instr,"LBU")==0) {
        printf("%s %s, %d (%s)\n",getInstr(u), getR(getRT(u)), getOffset(u), getR(getRS(u)));
    }
    else if(strcmp(instr,"LUI")==0) {
        printf("%s %s, %d\n",getInstr(u), getR(getRT(u)), getIm(u));
    }
    else if(strcmp(instr,"LW")==0) {
        printf("%s %s, %d(%s)\n",getInstr(u), getR(getRT(u)), getOffset(u), getR(getRS(u)));
    }
    else if(strcmp(instr,"MFHI")==0) {
        printf("%s %s\n",getInstr(u), getR(getRD(u)));
    }
    else if(strcmp(instr,"MFLO")==0) {
        printf("%s %s\n",getInstr(u), getR(getRD(u)));
    }
    else if(strcmp(instr,"MULT")==0) {
        printf("%s %s, %s\n",getInstr(u), getR(getRS(u)), getR(getRT(u)));
    }
    /* NOP */
    else if(strcmp(instr,"SLL")==0 && getRD(u)==0 && getRT(u)==0 && getSA(u)==0) {
        printf("NOP\n");
    }
    /*******/
    else if(strcmp(instr,"OR")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"ORI")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRT(u)), getR(getRS(u)), getIm(u));
    }
    else if(strcmp(instr,"SB")==0) {
        printf("%s %s, %d(%s)\n",getInstr(u), getR(getRT(u)), getOffset(u), getR(getRS(u)));
    }
    else if(strcmp(instr,"SEB")==0) {
        printf("%s %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"SLL")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRT(u)), getR(getSA(u)));
    }
    else if(strcmp(instr,"SLT")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"SLTI")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRT(u)), getR(getRS(u)), getIm(u));
    }
    else if(strcmp(instr,"SLTIU")==0) {
        printf("%s %s, %s, %d\n",getInstr(u), getR(getRT(u)), getR(getRS(u)), getIm(u));
    }
    else if(strcmp(instr,"SLTU")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"SRA")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRT(u)), getR(getSA(u)));
    }
    else if(strcmp(instr,"SRL")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRT(u)), getR(getSA(u)));
    }
    else if(strcmp(instr,"SUB")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"SUBU")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else if(strcmp(instr,"SW")==0) {
        printf("%s %s, %d(%s)\n",getInstr(u), getR(getRT(u)), getOffset(u), getR(getRS(u)));
    }
    else if(strcmp(instr,"SYSCALL")==0) {
        printf("%s\n",getInstr(u));
    }
    else if(strcmp(instr,"XOR")==0) {
        printf("%s %s, %s, %s\n",getInstr(u), getR(getRD(u)), getR(getRS(u)), getR(getRT(u)));
    }
    else printf("Instruction inconnue !!!\n");
}
