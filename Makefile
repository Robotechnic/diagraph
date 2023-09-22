wasm :
	cd graphviz_interface; \
	make -j16

compile_database:
	bear --output ./graphviz_interface/compile_commands.json -- make wasm

link :
	mkdir -p ~/.cache/typst/packages/preview/typst-graphviz
	ln -s "$(CURDIR)" ~/.cache/typst/packages/preview/typst-graphviz/0.1.0

module :
	mkdir -p ./graphviz
	cp ./graphviz.wasm ./graphviz/graphviz.wasm
	cp ./lib.typ ./graphviz/lib.typ
	cp ./README.MD ./graphviz/README.MD
	cp ./typst.toml ./graphviz/typst.toml
	cp ./LICENSE ./graphviz/LICENSE

clean :
	cd graphviz_interface; \
	make clean
	rm -rf ~/.cache/typst/packages/preview/graphviz