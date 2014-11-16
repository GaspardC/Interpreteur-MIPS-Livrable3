
#ifndef MIPS_ELF_TEST_H
#define MIPS_ELF_TEST_H


int is_in_symbols(char* name, stab symtab);
unsigned int get_nsegments(stab symtab,char* section_names[],int nb_sections);
int elf_load_section_in_memory(FILE* fp, mem memory, char* scn,unsigned int permissions,unsigned long long add_start);
void print_segment_raw_content(segment* seg);


#endif
