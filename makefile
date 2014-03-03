make:
	gcc -c strlcpy.c
	gcc -o myserver myserver.c myserver.h

run:
	./myserver 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/testLogFile.txt

runbadport:
	./myserver five /static/files/ logfile.txt

runbaddir:
	./myserver 8000 static/dirthatdoesntexist logfile.txt

runbadlog:
	./myserver 8000 /Users/Eric/git/379/assignment2/static/filesToServe /Users/Eric/git/379/assignment2/logfiledoesntexist.txt

clean:
	rm -f *.o *~ core

