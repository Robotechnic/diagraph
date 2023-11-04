wasm :
	cd graphviz_interface; \
	make -j16

compile_database:
	bear --output ./graphviz_interface/compile_commands.json -- make wasm

link :
	mkdir -p ~/.cache/typst/packages/preview/diagraph
	ln -s "$(CURDIR)" ~/.cache/typst/packages/preview/diagraph/0.1.2

module :
	mkdir -p ./diagraph
	cp ./diagraph.wasm ./diagraph/diagraph.wasm
	cp ./lib.typ ./diagraph/lib.typ
	cp ./README.MD ./diagraph/README.MD
	cp ./typst.toml ./diagraph/typst.toml
	cp ./LICENSE ./diagraph/LICENSE

clean : clean-link
	cd graphviz_interface; \
	make clean

clean-link:
	rm -rf ~/.cache/typst/packages/preview/diagraph

wasi-stub:
	git clone -n --depth=1 --filter=tree:0 https://github.com/astrale-sharp/wasm-minimal-protocol.git
	cd wasm-minimal-protocol; \
	git sparse-checkout set --no-cone wasi-stub; \
	git checkout
	cd wasm-minimal-protocol/wasi-stub; \
	cargo install --path . 
	rm -rf wasm-minimal-protocol