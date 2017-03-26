warmup2: warmup2.o serverThread.o packetCreationThread.o tokenGeneratingThread.o my402list.o signalHandler.o
	gcc -o warmup2 -g warmup2.o serverThread.o packetCreationThread.o tokenGeneratingThread.o my402list.o signalHandler.o -lpthread -lm -D_POSIX_PTHREAD_SEMANTICS

warmup2.o: warmup2.c my402list.h definitions.h
	gcc -g -c -Wall warmup2.c

packetCreationThread.o: packetCreationThread.c my402list.h definitions.h
	gcc -g -c -Wall packetCreationThread.c

serverThread.o: serverThread.c my402list.h definitions.h
	gcc -g -c -Wall serverThread.c
	
tokenGeneratingThread.o: tokenGeneratingThread.c my402list.h definitions.h
	gcc -g -c -Wall tokenGeneratingThread.c
		
my402list.o: my402list.c my402list.h
	gcc -g -c -Wall my402list.c

signalHandler.o: signalHandler.c definitions.h
	gcc -g -c -Wall signalHandler.c

clean:
	rm -f *.o warmup2
