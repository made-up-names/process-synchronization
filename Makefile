all:
	gcc -pthread -o badminton q1busywait.c
	gcc -pthread -o pollingbooth evmpollingbooth.c
	gcc -pthread -o normalmerge mnormal.c
	gcc -pthread -o threadmerge mthread.c
	gcc -pthread -o forkmerge mprocess.c

clean:
	rm badminton pollingbooth normalmerge threadmerge forkmerge
