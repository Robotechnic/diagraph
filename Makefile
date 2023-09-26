wasm :
	cd graphviz_interface; \
	make -j16

compile_database:
	bear --output ./graphviz_interface/compile_commands.json -- make wasm

link :
	mkdir -p ~/.cache/typst/packages/preview/diagraph
	ln -s "$(CURDIR)" ~/.cache/typst/packages/preview/diagraph/0.1.1

module :
	mkdir -p ./diagraph
	cp ./diagraph.wasm ./diagraph/diagraph.wasm
	cp ./lib.typ ./diagraph/lib.typ
	cp ./README.MD ./diagraph/README.MD
	cp ./typst.toml ./diagraph/typst.toml
	cp ./LICENSE ./diagraph/LICENSE

clean :
	cd graphviz_interface; \
	make clean
	rm -rf ~/.cache/typst/packages/preview/diagraph