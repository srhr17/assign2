buffer: test_assign2_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o
	gcc -o buffer test_assign2_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o
test_assign2_1.o: test_assign2_1.c
	gcc -c test_assign2_1.c
buffer_mgr.o: buffer_mgr.c
	gcc -c buffer_mgr.c
storage_mgr.o: storage_mgr.c
	gcc -c storage_mgr.c
dberror.o: dberror.c
	gcc -c dberror.c
buffer_mgr_stat.o: buffer_mgr_stat.c
	gcc -c buffer_mgr_stat.c
run: buffer
	./buffer
clean:
	rm buffer test_assign2_1.o buffer_mgr.o storage_mgr.o dberror.o buffer_mgr_stat.o
