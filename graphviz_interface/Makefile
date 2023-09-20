# setup for the C graphiz interface

CC=emcc
CONFIGURE=emconfigure
EMMAKE=emmake
SOURCES=main.c
INSTALLDIR=$(shell pwd)/install
SRCDIR=graphiz-src
EPSRCDIR=expat-src
CFLAGS = -Oz --closure=0 --no-entry -sERROR_ON_UNDEFINED_SYMBOLS=0 -sFILESYSTEM=0 -sASSERTIONS=0 -sEXPORT_KEEPALIVE=1
INCLUDE_FLAGS = -I"$(INSTALLDIR)/include" -I"$(INSTALLDIR)/include/graphviz" -L"$(INSTALLDIR)/lib" -L"$(INSTALLDIR)/lib/graphviz" -lgvplugin_dot_layout -lgvplugin_neato_layout -lgvplugin_core -lgvc -lpathplan -lcgraph -lxdot -lcdt -lexpat

all: build

build: $(SOURCES) | $(INSTALLDIR)/lib/libexpat.a $(INSTALLDIR)/lib/graphviz
	$(CC) $(CFLAGS) $(SOURCES) -o graphviz.wasm $(INCLUDE_FLAGS)
	wasi-stub ./graphviz.wasm -o ./graphviz.wasm
	mv ./"graphviz - stubbed.wasm" ../graphviz.wasm

clean:
	rm -rf $(SRCDIR)
	rm -rf $(EPSRCDIR)
	rm -rf $(INSTALLDIR)
	rm *.wasm || true
	rm libexpat-src.tar.gz || true
	rm graphviz-src.tar.gz || true

$(INSTALLDIR)/lib/graphviz: $(SRCDIR) $(INSTALLDIR) | $(SRCDIR)/Makefile
	cd $(SRCDIR)/lib && emmake make install -j16
	cd $(SRCDIR)/plugin && emmake make install -j16

$(SRCDIR)/Makefile: | $(SRCDIR) $(INSTALLDIR) $(INSTALLDIR)/lib/libexpat.a 
	cd $(SRCDIR) \
	&& emconfigure ./configure --host=wasm32 --disable-ltdl \
		--prefix="$(INSTALLDIR)" EXPAT_CFLAGS="-I$(INSTALLDIR)/include" EXPAT_LIBS="-L$(INSTALLDIR)/lib -lexpat" CFLAGS="-Oz" CXXFLAGS="-Oz"
		--enable-swig=no --enable-sharp=no --enable-go=no \
		--enable-ocaml=no --enable-perl=no --enable-php=no \
		--enable-python=no --enable-r=no --enable-ruby=no \

$(INSTALLDIR)/lib/libexpat.a: $(EPSRCDIR) $(INSTALLDIR) | $(EPSRCDIR)/Makefile
	cd "$(EPSRCDIR)/lib" && emmake make all install -j16

$(EPSRCDIR)/Makefile: | $(EPSRCDIR) $(INSTALLDIR)
	cd $(EPSRCDIR) && $(CONFIGURE) ./configure --prefix="$(INSTALLDIR)" \
		--host=wasm32 --disable-shared CFLAGS="-Oz" CXXFLAGS="-Oz"

$(INSTALLDIR):
	mkdir -p $(INSTALLDIR)

# download and extract graphviz source
$(SRCDIR): | graphviz-src.tar.gz
	mkdir -p $(SRCDIR)
	tar xvzf graphviz-src.tar.gz -C $(SRCDIR) --strip=1

graphviz-src.tar.gz:
	curl "https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/9.0.0/graphviz-9.0.0.tar.gz" -o graphviz-src.tar.gz

# download and extract expat source
$(EPSRCDIR): | libexpat-src.tar.gz
	mkdir -p $(EPSRCDIR)
	tar xvzf libexpat-src.tar.gz -C $(EPSRCDIR) --strip=1

libexpat-src.tar.gz:
	curl "https://netix.dl.sourceforge.net/project/expat/expat/2.5.0/expat-2.5.0.tar.gz" -o libexpat-src.tar.gz