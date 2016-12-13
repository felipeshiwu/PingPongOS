PingpongOS: ppos_core.c queue.c
	gcc -Wall -o PPOS ppos_core.c queue.c pingpong-prodcons.c -g

clean:
	@rm -rf *.o PPOS
