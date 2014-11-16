# fichier de test

load boucle.o
disp mem map
disp mem 0x3000:0x3030
disasm 0x3000:0x3030

set reg $1 10
disp reg all
debug
set reg $1 1
disp reg $1
