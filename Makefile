# objects = src/main.o src/system.o src/auth.o

# atm : $(objects)
#	cc -o atm $(objects)

#main.o : src/header.h
#kbd.o : src/header.h
#command.o : src/header.h
#display.o : src/header.h
#insert.o : src/header.h
#search.o : src/header.h
#files.o : src/header.h
#utils.o : src/header.h

#clean :
#	rm -f $(objects)


objects = src/main.o src/system.o src/auth.o

atm: $(objects)
	cc -o atm $(objects)

src/main.o: src/main.c src/header.h
	cc -c src/main.c -o src/main.o

src/system.o: src/system.c src/header.h
	cc -c src/system.c -o src/system.o

src/auth.o: src/auth.c src/header.h
	cc -c src/auth.c -o src/auth.o

.PHONY: clean
clean:
	rm -f $(objects) atm