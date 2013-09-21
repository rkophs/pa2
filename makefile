all:
	gcc GBNclient.c -o clientDir/client
	gcc GBNserver.c -o serverDir/server

clean:
	rm -rf clientDir/client serverDir/server

superclean:
	rm -rf clientDir/ serverDir/
