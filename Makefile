# -*- MakeFile -*-

all: P5
	@echo " " 
	@echo "Compilação concluída!!"
	@echo " " 
	 
P5: main.o gerador_de_testes.o imagem.o projeto5.o
	gcc -o P5 main.o gerador_de_testes.o imagem.o projeto5.o

main.o: main.c
	gcc -o main.o -c main.c

gerador_de_testes.o: gerador_de_testes.c gerador_de_testes.h
	gcc -o gerador_de_testes.o -c gerador_de_testes.c

imagem.o: imagem.c imagem.h
	gcc -o imagem.o -c imagem.c

projeto5.o: projeto5.c
	gcc -o projeto5.o -c projeto5.c  

clean: 
	rm -f *.o

execClean:
	rm -f saida