#include "asm.h"
#include "registre.h"
#include "lib.h"


int execute_asm(uint32_t u, registre r, mem memory)
{
    char * instr = getInstr(u);  
    if(strcmp(instr,"ADD")==0) {
    	uint64_t temp=0;
	temp  = ((uint64_t)getbits(r->reg[getRS(u)],31,31) << 32) | (uint64_t)r->reg[getRS(u)];
	temp += ((uint64_t)getbits(r->reg[getRT(u)],31,31) << 32) | (uint64_t)r->reg[getRT(u)];
	
	if ((temp & 0x100000000)>>32 != (temp & 0x80000000)>>31) {
		WARNING_MSG("Overflow");
		return 1;
	} 
	else {
		r->reg[getRD(u)] =  r->reg[getRS(u)] + r->reg[getRT(u)];
		return 0;
	}
    }
    
    else if(strcmp(instr,"ADDI")==0) {
        uint64_t temp=0;
	temp  = ((uint64_t)getbits(r->reg[getRS(u)],31,31) << 32) | (uint64_t)r->reg[getRS(u)];
	temp += ((uint64_t)getbits((uint64_t)(getIm(u)),31,31) << 32) | (int64_t)(getIm(u));
	
	if ((temp & 0x100000000)>>32 != (temp & 0x80000000)>>31) {
		WARNING_MSG("Overflow");
		return 1;
	} 
	else {
		r->reg[getRT(u)] =  r->reg[getRS(u)] + (int32_t)(getIm(u));
		return 0;
	}
    }
    
    else if(strcmp(instr,"ADDIU")==0) {
        r->reg[getRT(u)] =  r->reg[getRS(u)] + (int32_t)(getIm(u));
	return 0;
    }
    
    else if(strcmp(instr,"ADDU")==0) {
        r->reg[getRD(u)] =  r->reg[getRS(u)] + r->reg[getRT(u)];
	return 0;
    }
    
    else if(strcmp(instr,"AND")==0) {
        r->reg[getRD(u)] =  r->reg[getRS(u)] & r->reg[getRT(u)];
	return 0;
    }
    
    else if(strcmp(instr,"ANDI")==0) {
        r->reg[getRT(u)] =  r->reg[getRS(u)] & (int32_t)(getIm(u));
	return 0;
    }
    
    else if(strcmp(instr,"BEQ")==0) {
	int64_t target_offset;
	int condition;
	/* I: */   condition = (r->reg[getRS(u)] == r->reg[getRT(u)]);
	           target_offset = (int64_t)(getOffset(u) << 2);
	//NextInstruction();
	/* I+1: */ if (condition) {
			r->reg[32] = r->reg[32] + target_offset - 4;
		   }
	return 0;
    }
    
    else if(strcmp(instr,"BGEZ")==0) {
	int64_t target_offset;
	int condition;
	/* I: */   condition = (r->reg[getRS(u)] >= 0);
	           target_offset = (int64_t)(getOffset(u) << 2);
	//NextInstruction();
	/* I+1: */ if (condition) {
			r->reg[32] = r->reg[32] + target_offset - 4;
		   }
	return 0;
    }
    
    else if(strcmp(instr,"BGTZ")==0) {
	int64_t target_offset;
	int condition;
	/* I: */   condition = (r->reg[getRS(u)] > 0);

	           target_offset = (int64_t)(getOffset(u) << 2);
	//NextInstruction();
	/* I+1: */ if (condition) {
			r->reg[32] = r->reg[32] + target_offset - 4;
		   }
	return 0;
    }
    
    else if(strcmp(instr,"BLEZ")==0) {
	int64_t target_offset;
	int condition;
	/* I: */   condition = (r->reg[getRS(u)] <= 0);
	           target_offset = (int64_t)(getOffset(u) << 2);
	//NextInstruction();
	/* I+1: */ if (condition) {
			r->reg[32] = r->reg[32] + target_offset - 4;
		   }
	return 0;
    }
    
    else if(strcmp(instr,"BLTZ")==0) {
	int64_t target_offset;
	int condition;
	/* I: */   condition = (r->reg[getRS(u)] < 0);
	           target_offset = (int64_t)(getOffset(u) << 2);
	//NextInstruction(); 
	/* I+1: */ if (condition) {
			r->reg[32] = r->reg[32] + target_offset - 4;
		   }
	return 0;
    }
    
    else if(strcmp(instr,"BNE")==0) {
	int64_t target_offset;
	int condition;
	/* I: */   condition = (r->reg[getRS(u)] != r->reg[getRT(u)]);
	           target_offset = (int64_t)(getOffset(u) << 2);
	//NextInstruction();
	/* I+1: */ if (condition) {
			r->reg[32] = r->reg[32] + target_offset - 4;
		   }
	return 0;
    }
    
    else if(strcmp(instr,"BREAK")==0) {
        return BREAK;
    }
    
    else if(strcmp(instr,"DIV")==0) {
        r->reg[34]=(int32_t)r->reg[getRS(u)] / (int32_t)r->reg[getRT(u)];
        r->reg[33]=(int32_t)r->reg[getRS(u)] % (int32_t)r->reg[getRT(u)];
        return 0;
    }
    
    else if(strcmp(instr,"J")==0) {
        /* I: */ 
	//NextInstruction();
	/* I+1: */ r->reg[32] = ((r->reg[32] & 0xF0000000)>>28) | (getTarget(u) << 2);
	return 0;
    }
    
    else if(strcmp(instr,"JAL")==0) {
        /* I: */r->reg[31]=r->reg[32]+8;
	//NextInstruction();
	/* I+1: */ r->reg[32] = ((r->reg[32] & 0xF0000000)>>28) | (getTarget(u) << 2);
	return 0;
    }
    
    else if(strcmp(instr,"JARL")==0) {
        uint32_t temp;
	/* I: */   temp = r->reg[getRS(u)];
		   r->reg[getRD(u)] = r->reg[32] + 8;
	//NextInstruction();
	/* I+1: */ r->reg[32] = temp;
    }
    
    else if(strcmp(instr,"JR")==0) {
        uint32_t temp;
	/* I: */   temp = r->reg[getRS(u)];
	//NextInstruction();
	/* I+1: */ r->reg[32] = temp;
    }
    
    else if(strcmp(instr,"LB")==0) {
        uint32_t vAddr = (int32_t)(getOffset(u))+r->reg[getRS(u)];
	uint32_t byte = (int32_t)loadmem(vAddr, memory, "BYTE");
	r->reg[getRT(u)] = byte;
	return 0;
    }
    
    else if(strcmp(instr,"LBU")==0) {
        uint32_t vAddr = (int32_t)(getOffset(u))+r->reg[getRS(u)];
	r->reg[getRT(u)] = loadmem(vAddr, memory, "BYTE");
	return 0;
    }
    
    else if(strcmp(instr,"LUI")==0) {
        r->reg[getRT(u)] = getIm(u)<<16;
	return 0;
    }
    
    else if(strcmp(instr,"LW")==0) {
        uint32_t vAddr = (int32_t)(getOffset(u))+r->reg[getRS(u)];
        if (getbits(vAddr,0,1) != 0) {
		WARNING_MSG("Probleme d'adresse");
		return 1;
	}
	int32_t word = loadmem(vAddr, memory, "WORD");
	r->reg[getRT(u)] = word;
	return 0;
	
    }
    else if(strcmp(instr,"MFHI")==0) {
        r->reg[getRD(u)] =  r->reg[33];
	return 0;
    }
    
    else if(strcmp(instr,"MFLO")==0) {
        r->reg[getRD(u)] =  r->reg[34];
	return 0;
    }
    
    else if(strcmp(instr,"MULT")==0) {
        uint64_t prod=r->reg[getRS(u)]*r->reg[getRT(u)];
        r->reg[34]=prod & 0x00000000FFFFFFFF;
        r->reg[33]=(prod & 0xFFFFFFFF00000000)>>32;
        return 0;
    }
    
    else if(strcmp(instr,"SLL")==0 && getRD(u)==0 && getRT(u)==0 && getSA(u)==0) {
        return 0;
    }
    
    else if(strcmp(instr,"OR")==0) {
        r->reg[getRD(u)] =  r->reg[getRS(u)] | r->reg[getRT(u)];
	return 0;
    }
    
    else if(strcmp(instr,"ORI")==0) {
        r->reg[getRT(u)] =  r->reg[getRS(u)] | (int32_t)(getIm(u));
	return 0;
    }
    /*else if(strcmp(instr,"SB")==0) {
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
    */else if(strcmp(instr,"XOR")==0) {
        r->reg[getRD(u)] =  r->reg[getRS(u)] ^ r->reg[getRT(u)];
	return 0;
    }
    else {
    	WARNING_MSG("Instruction inconnue");
    	return 2;
    }

}



/* LOAD MEMORY */

int loadmem(uint32_t vAddr, mem memory, char* type)
{
	if(memory==NULL)
	{
		WARNING_MSG("Memoire vide");
		return 0;
	}
	
	if(strcmp(type,"WORD")==0)
	{
		int n=0;
		uint32_t combined=0;
		combined = (*(memory->seg[n].content + (vAddr - memory->seg[n].start._64))) << 24 | (*(memory->seg[n].content + (vAddr + 1 - memory->seg[n].start._64))) << 16 | (*(memory->seg[n].content + (vAddr + 2 - memory->seg[n].start._64))) << 8 | (*(memory->seg[n].content + (vAddr + 3 - memory->seg[n].start._64)));
		return combined;
	}
	if(strcmp(type,"BYTE")==0)
	{
		int n=0;
		uint8_t combined=0;
		combined = (*(memory->seg[n].content + (vAddr - memory->seg[n].start._64)));
		return combined;
	}
	else return 0;
}
