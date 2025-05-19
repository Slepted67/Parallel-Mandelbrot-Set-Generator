CC = mpicc
CFLAGS = -fopenmp `pkg-config --cflags opencv4`
LDFLAGS = -fopenmp `pkg-config --libs opencv4`
PROG = mandelbrot.x
OBJS = mandelbrot.o imageTools.o

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROG)
