make:
	gcc -c strlcpy.c
	gcc -o myserver myserver.c myserver.h

makef:
	gcc -c strlcpy.c
	gcc -o server_f server_f.c server_f.h

makep:
	gcc -c strlcpy.c
	gcc -o server_p server_p.c server_p.h -pthread

makes:
	gsc -c strlcpy.c
	gcc -o server_s server_s.c server_s.h

run:
	./myserver 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/testLogFile.txt

runf:
	./server_f 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/testLogFile.txt

runp:
	./server_p 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/testLogFile.txt

runs:
	./server_s 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/testLogFile.txt

runbadport:
	./myserver five /static/files/ logfile.txt

runbaddir:
	./myserver 8000 static/dirthatdoesntexist logfile.txt

runbadlog:
	./myserver 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/logfiledoesntexist.txt

clean:
	rm -f *.o *~ core

