#ifndef REGISTRE_H
#define REGISTRE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef int32_t reg;

// définition des registres
typedef struct {
	size_t nreg;
	reg *reg;
} *registre;

registre registre_new(size_t nbreg);

void registre_del(registre c);

int convert(char *regName);

int getRegisterValue(registre r, int numReg);

int getRegisterValueByStr(registre r, char *regName);

int setRegisterValue(registre r, int numReg, int value);

char* getName(char* regName);

char* getName2(int a);

#endif
