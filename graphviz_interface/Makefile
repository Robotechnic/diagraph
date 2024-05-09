# setup for the C graphiz interface

CC=emcc
CONFIGURE=emconfigure
EMMAKE=emmake
SOURCES=src/main.c $(wildcard src/*/*.c)
INSTALLDIR=$(shell pwd)/install
SRCDIR=$(shell pwd)/graphiz-src
EPSRCDIR=$(shell pwd)/expat-src
CFLAGS = -Oz --no-entry -sERROR_ON_UNDEFINED_SYMBOLS=0 -sFILESYSTEM=0 -sASSERTIONS=0 -sEXPORT_KEEPALIVE=1 -Wall -Wno-logical-op-parentheses
INCLUDE_FLAGS = -I"src" -I"$(INSTALLDIR)/include" -I"$(INSTALLDIR)/include/graphviz" -L"$(INSTALLDIR)/lib" -L"$(INSTALLDIR)/lib/graphviz" -lgvplugin_dot_layout -lgvplugin_neato_layout -lgvplugin_core -lgvc -lpathplan -lcgraph -lxdot -lcdt -lexpat -lm
GRAPHVIZ_INFO_INCLUDE=-include "$(SRCDIR)/graphviz_version.h"
INCLUDE_FLAGS += $(GRAPHVIZ_INFO_INCLUDE)

all: build

build: $(SOURCES) $(INSTALLDIR)/lib/libexpat.a $(INSTALLDIR)/lib/graphviz src/protocol/protocol.c
	$(CC) $(CFLAGS) $(SOURCES) -o diagraph.wasm $(INCLUDE_FLAGS)
	wasi-stub --stub-function env:__syscall_unlinkat,env:__syscall_faccessat ./diagraph.wasm
	mv ./"diagraph - stubbed.wasm" ./diagraph.wasm

protocol src/protocol/protocol.c src/protocol/protocol.h: diagraph.prot
	wasmpg diagraph.prot -t . -c src/protocol

format:
	clang-format -i -style=file src/*.c src/*/*.c src/*/*.h

clean:
	rm -rf $(SRCDIR)
	rm -rf $(EPSRCDIR)
	rm -rf $(INSTALLDIR)
	rm -f *.wasm \
		  libexpat-src.tar.xz \
		  graphviz-src.tar.gz \
		  src/protocol/protocol.c \
		  src/protocol/protocol.h

$(INSTALLDIR)/lib/graphviz: $(SRCDIR) $(INSTALLDIR) $(SRCDIR)/Makefile
	cd $(SRCDIR)/lib && emmake make install -j16
	cd $(SRCDIR)/plugin && emmake make install -j16

$(SRCDIR)/Makefile: | $(SRCDIR) $(INSTALLDIR) $(INSTALLDIR)/lib/libexpat.a 
	cd $(SRCDIR) \
	&& emconfigure ./configure --host=wasm32 --disable-ltdl \
		--prefix="$(INSTALLDIR)" EXPAT_CFLAGS="-I$(INSTALLDIR)/include" EXPAT_LIBS="-L$(INSTALLDIR)/lib -lexpat" CFLAGS="-Oz" CXXFLAGS="-Oz"
		--enable-swig=no --enable-sharp=no --enable-go=no \
		--enable-ocaml=no --enable-perl=no --enable-php=no \
		--enable-python=no --enable-r=no --enable-ruby=no \
		--disable-man-pdfs --disable-guile --diable-java \
		--disable-tcl

$(INSTALLDIR)/lib/libexpat.a: $(EPSRCDIR) $(INSTALLDIR) | $(EPSRCDIR)/Makefile
	cd "$(EPSRCDIR)/lib" && emmake make all install -j16

$(EPSRCDIR)/Makefile: | $(EPSRCDIR)/configure $(INSTALLDIR)
	cd $(EPSRCDIR) && $(CONFIGURE) ./configure --prefix="$(INSTALLDIR)" \
		--host=wasm32 --disable-shared CFLAGS="-Oz" CXXFLAGS="-Oz"

$(INSTALLDIR):
	mkdir -p $(INSTALLDIR)

# download and extract graphviz source
$(SRCDIR): | graphviz-src.tar.gz
	mkdir -p $(SRCDIR)
	tar xvzf graphviz-src.tar.gz -C $(SRCDIR) --strip=1

graphviz-src.tar.gz:
	curl "https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/11.0.0/graphviz-11.0.0.tar.gz" -o graphviz-src.tar.gz

# download and extract expat source
$(EPSRCDIR) $(EPSRCDIR)/configure: | libexpat-src.tar.xz
	mkdir -p $(EPSRCDIR)
	tar xf libexpat-src.tar.xz -C $(EPSRCDIR) --strip=1

libexpat-src.tar.xz:
	curl "https://netix.dl.sourceforge.net/project/expat/expat/2.6.2/expat-2.6.2.tar.xz" -o libexpat-src.tar.xz