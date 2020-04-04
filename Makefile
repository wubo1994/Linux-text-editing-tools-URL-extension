all: urlextend

urlextend: urlextend.c
	gcc urlextend.c -o urlextend
