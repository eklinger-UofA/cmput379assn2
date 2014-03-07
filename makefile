make:
	gcc -c strlcpy.c
	gcc -o myserver myserver.c myserver.h strlcpy.o

makef:
	cc -m32 -c strlcpy.c
	cc -m32 -o server_f server_f.c server_f.h strlcpy.o

makep:
	cc -m32 -c strlcpy.c
	cc -m32 -o server_p server_p.c server_p.h  strlcpy.o -lpthread

makes:
	cc -m32 -c strlcpy.c
	cc -m32 -o server_s server_s.c server_s.h strlcpy.o

make all:
	cc -m32 -c strlcpy.c
	cc -m32 -o server_f server_f.c server_f.h strlcpy.o
	cc -m32 -o server_p server_p.c server_p.h  strlcpy.o -lpthread
	cc -m32 -o server_s server_s.c server_s.h strlcpy.o

run:
	./myserver 8000 /Users/Eric/git/379/assignment2/notforsubmission/static/filesToServe /Users/Eric/git/379/assignment2/notforsubmission/testLogFile.txt

runf:
	./server_f 8000 /cshome/eklinger/Documents/year5sem2/cmput379/assignment2/notforsubmission/static/filesToServe /cshome/eklinger/Documents/year5sem2/cmput379/assignment2/notforsubmission/testLogFile.txt

runp:
	./server_p 8001 /cshome/eklinger/Documents/year5sem2/cmput379/assignment2/notforsubmission/static/filesToServe /cshome/eklinger/Documents/year5sem2/cmput379/assignment2/notforsubmission/testLogFile.txt
runs:
	./server_s 8002 /cshome/eklinger/Documents/year5sem2/cmput379/assignment2/notforsubmission/static/filesToServe /cshome/eklinger/Documents/year5sem2/cmput379/assignment2/notforsubmission/testLogFile.txt
runbadport:
	./myserver five /static/files/ logfile.txt

runbaddir:
	./myserver 8000 static/dirthatdoesntexist logfile.txt

runbadlog:
	./myserver 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/logfiledoesntexist.txt

clean:
	rm -f *.o *~ core

