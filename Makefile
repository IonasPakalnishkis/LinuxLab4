main:
	gcc -Wall lab4.c -lfuse -o lab4 -D_FILE_OFFSET_BITS=64
	mkdir root_dir
	./lab4 root_dir

clean:
	rm lab4
	fusermount -u root_dir
	rm -r root_dir
