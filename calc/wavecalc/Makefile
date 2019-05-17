APPNAME = wavecalc
SOURCES = main.c

BUILDDIR = build
OBJECTS   = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))

CC = gcc
CFLAGS = -O3 -c -DT_LINUX
#CFLAGS = -std=c99 -pedantic -Wall


all: $(APPNAME)

$(BUILDDIR)/%.o: %.c $(wildcard *.h)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(APPNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(APPNAME)

clean:
	rm -f $(BUILDDIR)/*.o $(APPNAME)
