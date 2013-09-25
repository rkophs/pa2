all:
	mkdir -p clientDir
	cp -f transferFiles/ocean.jpg clientDir/ocean.jpg
	cp -f transferFiles/lorem.txt clientDir/lorem.txt
	cp -f transferFiles/foo2 clientDir/foo2
	mkdir -p serverDir
	gcc -g GBNclient.c -o clientDir/client
	gcc -g GBNserver.c -o serverDir/server
	gcc -g test.c -o test

clean:
	rm -rf clientDir/ serverDir/
	rm test
