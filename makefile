make:
	gcc -o myserver myserver.c myserver.h

run:
	./myserver 8000 /static/filesToServe logfile.txt

runbadport:
	./myserver five /static/files/ logfile.txt

runbaddir:
	./myserver 8000 static/dirthatdoesntexist logfile.txt


clean:
	rm -f *.o *~ core

