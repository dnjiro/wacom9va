CC	=	gcc
CXX	=	g++ 


CFLAGS =	`pkg-config --cflags gtk+-3.0`
LDFLAGS =	`pkg-config --libs gtk+-3.0`    -lm 

SOURCES =	main.c 



OBJECTS =	$(SOURCES:.c=.o)
TARGET=	wacom9va

#This rule will be  executed for each file. '-c' is necessary to get '.o'
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all:	$(TARGET)

$(TARGET): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	-rm -f $(OBJECTS)  $(TARGET)
	-rm -f *~ core


