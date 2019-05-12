.DEFAULT_GOAL = all
.PHONY: all debug clean help folder

##########################################################
#                                                        #
#  Makefile generico para Redes II                       #
#                                                        #
#  BDIR - Carpeta donde se crearan los ejecutables       #
#                                                        #
#  SDIR - Carpeta con los ficheros fuente                #
#                                                        #
#  IDIR - Carpeta con todas las cabeceras                #
#                                                        #
#  ODIR - Carpeta con todos los objetos .o compilados    #
#                                                        #
#  EDIR - Carpeta con los mains                          #
#                                                        #
##########################################################

BDIR := .
SDIR := src
IDIR := include
ODIR := obj
EDIR := srclib

CC       ?= gcc
CFLAGS   := -Iinclude -ansi -pedantic -Wall -Wextra -std=c99 -g 
LDFLAGS  := -pthread -lconfuse
RM       := rm -fvd

## Especificamos los archivos a compilar en srclib
EXES := main.c

## Ficheros fuente objetivos del make all
EXES := $(patsubst %,$(EDIR)/%,$(EXES))
EOBJ := $(patsubst $(EDIR)/%.c,$(ODIR)/%.o,$(EXES))
EBIN := $(patsubst $(EDIR)/%.c,$(BDIR)/%,$(EXES))

## Fuentes del directorio SDIR
SRCS := $(filter-out $(EXES), $(wildcard $(SDIR)/*.c))
SOBJ := $(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SRCS))

## Flags especificos por objetivo
all: CFLAGS += -O3 -DNDEBUG
debug: CFLAGS += -g

all: folder $(EBIN)
debug: folder $(EBIN)

## Compilacion de .c de src
$(SOBJ):$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

## Compilacion de .c de exes
$(EOBJ):$(ODIR)/%.o: $(EDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

## Linkado de exes
$(EBIN):$(BDIR)/%: $(ODIR)/%.o $(SOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@$(RM) $(SOBJ) $(EOBJ) $(EBIN)
	@$(RM) $(ODIR) 2> /dev/null

help:
	@echo "Posible commands:"
	@echo "    all      - build the exe $(EBIN)"
	@echo "    debug    - compile with debugs"
	@echo "    clean    - cleans the objects"
	@echo "    help     - show help"

folder:
	@mkdir -p $(ODIR)
