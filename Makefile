wasm :
	cd graphviz_interface; \
	make -j16

link :
	mkdir -p ~/.cache/typst/packages/preview/typst-graphviz
	ln -s "$(CURDIR)" ~/.cache/typst/packages/preview/typst-graphviz/0.1.0

clean :
	cd graphviz_interface; \
	cargo clean
	rm -f graphviz.wasm
	rm -rf ~/.cache/typst/packages/preview/graphviz