all:
	mkdir -p clientDir
	cp -f ocean.jpg clientDir/ocean.jpg
	cp -f lorem.txt clientDir/lorem.txt
	cp -f foo2 clientDir/foo2
	mkdir -p serverDir
	gcc GBNclient.c -o clientDir/client
	gcc GBNserver.c -o serverDir/server

clean:
	rm -rf clientDir/ serverDir/
