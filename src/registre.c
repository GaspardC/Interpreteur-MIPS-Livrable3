
#include "registre.h"
#include "emulMips.h"

/*************************************************************\
fonctions de l'émulateur mips
\*************************************************************/

// initialisation registres
registre registre_new(size_t nbreg) {
    registre reg_emul=calloc(1, sizeof(*reg_emul));
    if(reg_emul==NULL)
        printf("impossible d'allouer les registres");

    reg_emul->reg = calloc(nbreg, sizeof(*(reg_emul->reg)));
    if(reg_emul->reg==NULL)
        printf("impossible d'allouer les registres");

    reg_emul->nreg = nbreg;

    return reg_emul;
}

// supprimer registres
void registre_del(registre c) {
    if(c->reg != NULL)
        free(c->reg);
    if(c != NULL)
        free(c);
}

// obtenir la valeur d'un numéro de registre
int getRegisterValue(registre r, int numReg) {
    if ( r != NULL && r->reg != NULL && numReg>=0 && numReg<=34)
        return r->reg[numReg];
    else return -1;
}

int convert(char *regName) {
    if (regName != NULL) {
        if(strcmp(regName,"$0")==0 || strcmp(regName,"$zero")==0)
            return 0;
        else if(strcmp(regName,"$1")==0 || strcmp(regName,"$at")==0)
            return 1;
        else if(strcmp(regName,"$2")==0 || strcmp(regName,"$v0")==0)
            return 2;
        else if(strcmp(regName,"$3")==0 || strcmp(regName,"$v1")==0)
            return 3;
        else if(strcmp(regName,"$4")==0 || strcmp(regName,"$a0")==0)
            return 4;
        else if(strcmp(regName,"$5")==0 || strcmp(regName,"$a1")==0)
            return 5;
        else if(strcmp(regName,"$6")==0 || strcmp(regName,"$a2")==0)
            return 6;
        else if(strcmp(regName,"$7")==0 || strcmp(regName,"$a3")==0)
            return 7;
        else if(strcmp(regName,"$8")==0 || strcmp(regName,"$t0")==0)
            return 8;
        else if(strcmp(regName,"$9")==0 || strcmp(regName,"$t1")==0)
            return 9;
        else if(strcmp(regName,"$10")==0 || strcmp(regName,"$t2")==0)
            return 10;
        else if(strcmp(regName,"$11")==0 || strcmp(regName,"$t3")==0)
            return 11;
        else if(strcmp(regName,"$12")==0 || strcmp(regName,"$t4")==0)
            return 12;
        else if(strcmp(regName,"$13")==0 || strcmp(regName,"$t5")==0)
            return 13;
        else if(strcmp(regName,"$14")==0 || strcmp(regName,"$t6")==0)
            return 14;
        else if(strcmp(regName,"$15")==0 || strcmp(regName,"$t7")==0)
            return 15;
        else if(strcmp(regName,"$16")==0 || strcmp(regName,"$s0")==0)
            return 16;
        else if(strcmp(regName,"$17")==0 || strcmp(regName,"$s1")==0)
            return 17;
        else if(strcmp(regName,"$31")==0 || strcmp(regName,"$ra")==0)
            return 31;
        else if(strcmp(regName,"$30")==0 || strcmp(regName,"$fp")==0)
            return 30;
        else if(strcmp(regName,"$29")==0 || strcmp(regName,"$sp")==0)
            return 29;
        else if(strcmp(regName,"$28")==0 || strcmp(regName,"$gp")==0)
            return 28;
        else if(strcmp(regName,"$27")==0 || strcmp(regName,"$k1")==0)
            return 27;
        else if(strcmp(regName,"$26")==0 || strcmp(regName,"$k0")==0)
            return 26;
        else if(strcmp(regName,"$25")==0 || strcmp(regName,"$t9")==0)
            return 25;
        else if(strcmp(regName,"$24")==0 || strcmp(regName,"$t8")==0)
            return 24;
        else if(strcmp(regName,"$23")==0 || strcmp(regName,"$s7")==0)
            return 23;
        else if(strcmp(regName,"$22")==0 || strcmp(regName,"$s6")==0)
            return 22;
        else if(strcmp(regName,"$21")==0 || strcmp(regName,"$s5")==0)
            return 21;
        else if(strcmp(regName,"$20")==0 || strcmp(regName,"$s4")==0)
            return 20;
        else if(strcmp(regName,"$19")==0 || strcmp(regName,"$s3")==0)
            return 19;
        else if(strcmp(regName,"$18")==0 || strcmp(regName,"$s2")==0)
            return 18;
        else if(strcmp(regName,"$32")==0 || strcmp(regName,"$PC")==0)
            return 32;
        else if(strcmp(regName,"$33")==0 || strcmp(regName,"$HI")==0)
            return 33;
        else if(strcmp(regName,"$34")==0 || strcmp(regName,"$LO")==0)
            return 34;
        else return -1;
    }

    else return -1;
}

// obtenir la valeur d'un numéro de registre à partir de son Mnémonique
int getRegisterValueByStr(registre r, char *regName) {
    int a = convert(regName);
    if(a!=-1)
        return r->reg[a];
    else return -1;
}

int setRegisterValue(registre r, int numReg, int value) {
    if ( r != NULL && r->reg != NULL && numReg>=0 && numReg<=34) {
        //printf("itoaa %s\n", itoa(value,16) );
        //r->reg[numReg]=itoa(value,16);
        r->reg[numReg]=value;
        return 0	;
    }

    else return -1;
}

char* getName(char* regName) {
    int a=convert(regName);
    if(a==0) return "zero";
    if(a==1) return "at";
    if(a==2) return "v0";
    if(a==3) return "v1";
    if(a==4) return "a0";
    if(a==5) return "a1";
    if(a==6) return "a2";
    if(a==7) return "a3";
    if(a==8) return "t0";
    if(a==9) return "t1";
    if(a==10) return "t2";
    if(a==11) return "t3";
    if(a==12) return "t4";
    if(a==13) return "t5";
    if(a==14) return "t6";
    if(a==15) return "t7";
    if(a==16) return "s0";
    if(a==17) return "s1";
    if(a==18) return "s2";
    if(a==19) return "s3";
    if(a==20) return "s4";
    if(a==21) return "s5";
    if(a==22) return "s6";
    if(a==23) return "s7";
    if(a==24) return "t8";
    if(a==25) return "t9";
    if(a==26) return "k0";
    if(a==27) return "k1";
    if(a==28) return "gp";
    if(a==29) return "sp";
    if(a==30) return "fp";
    if(a==31) return "ra";
    if(a==32) return "PC";
    if(a==33) return "HI";
    if(a==34) return "LO";

    else return "problème dans l'affichage du nom du registre";


}

char* getName2(int a) {
    if(a==0) return "zero";
    if(a==1) return "at";
    if(a==2) return "v0";
    if(a==3) return "v1";
    if(a==4) return "a0";
    if(a==5) return "a1";
    if(a==6) return "a2";
    if(a==7) return "a3";
    if(a==8) return "t0";
    if(a==9) return "t1";
    if(a==10) return "t2";
    if(a==11) return "t3";
    if(a==12) return "t4";
    if(a==13) return "t5";
    if(a==14) return "t6";
    if(a==15) return "t7";
    if(a==16) return "s0";
    if(a==17) return "s1";
    if(a==18) return "s2";
    if(a==19) return "s3";
    if(a==20) return "s4";
    if(a==21) return "s5";
    if(a==22) return "s6";
    if(a==23) return "s7";
    if(a==24) return "t8";
    if(a==25) return "t9";
    if(a==26) return "k0";
    if(a==27) return "k1";
    if(a==28) return "gp";
    if(a==29) return "sp";
    if(a==30) return "fp";
    if(a==31) return "ra";
    if(a==32) return "PC";
    if(a==33) return "HI";
    if(a==34) return "LO";

    else return "problème dans l'affichage du nom du registre";
}


