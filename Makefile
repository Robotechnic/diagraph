wasm :
	cd graphviz_interface; \
	make -j16

compile_database:
	bear --output ./graphviz_interface/compile_commands.json -- make wasm

format:
	cd graphviz_interface; \
	make format

link :
	mkdir -p ~/.cache/typst/packages/preview/diagraph
	ln -s "$(CURDIR)" ~/.cache/typst/packages/preview/diagraph/0.2.3

module :
	mkdir -p ./diagraph
	cp ./diagraph.wasm ./diagraph/diagraph.wasm
	cp ./internals.typ ./diagraph/internals.typ
	cp ./lib.typ ./diagraph/lib.typ
	cp ./README.md ./diagraph/README.md
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