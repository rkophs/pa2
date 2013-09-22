all:
	mkdir -p clientDir
	cp -f ocean.jpg clientDir/ocean.jpg
	mkdir -p serverDir
	gcc GBNclient.c -o clientDir/client
	gcc GBNserver.c -o serverDir/server

clean:
	rm -rf clientDir serverDir
