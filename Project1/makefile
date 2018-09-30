HEADERS = *.h
OBJECTS = main.o

default: program

%.o: %.c $(HEADERS)
    gcc -c $< -o $@

program: $(OBJECTS)
    gcc $(OBJECTS) -o $@

clean:
    -rm -f $(OBJECTS)
    -rm -f program