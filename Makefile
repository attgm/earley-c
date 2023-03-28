OBJS = Grammar.o Registration.o main.o ParseList.o
PROG = earley
CFLAGS = -O
CC   = gcc
LINK = $(CC) $(CFLAGS) -o $(PROG)
	
# x.o
.c.o:
	$(CC) -c $(CFLAGS) $<

all: $(OBJS)
	$(LINK) $(OBJS)

$(PROG) : $(OBJS)
	$(LINK) $(OBJS)

clean   : 
	rm *.o $(PROG)

# 
ParseList.o   : header.h ParseList.c
Registration.o: header.h Registration.c
Grammar.o     : header.h Grammar.c
main.o	      : header.h main.c