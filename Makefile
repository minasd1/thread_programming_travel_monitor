CC = gcc
tFiles = hashTable.c skipList.c bloomFilter.c pathsBST.c parentBST.c country_namesBST.c tMonitor_functions.c socket_handling.c list.c error_handling.c hash.c bitops.c
mFiles = hashTable.c BST.c country_namesBST.c skipList.c bloomFilter.c cyclicBuffer.c filePathsList.c hash.c bitops.c socket_handling.c
args = -g -Wall -lm -o

all: 
		$(CC) travelMonitorClient.c $(tFiles) $(args) travelMonitorClient -lpthread
		$(CC) monitorServer.c $(mFiles) $(args) monitorServer -lpthread

travelMonitor:
	$(CC) travelMonitorClient.c $(tFiles) $(args) travelMonitorClient

Monitor:
	$(CC) monitorServer.c $(mFiles) $(args) monitorServer

clean:
	if [ -f travelMonitorClient ]; then rm travelMonitorClient; fi;
	if [ -f monitorServer ]; then rm monitorServer; fi;