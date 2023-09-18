wasm :
	cd graphviz; \
	cargo build --target wasm32-wasi --release
	cp graphviz/target/wasm32-wasi/release/graphviz.wasm ./graphviz.wasm

clean :
	cd graphviz; \
	cargo clean
	rm -f graphviz.wasm