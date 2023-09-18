use wasm_minimal_protocol::*;

initiate_protocol!();

#[wasm_func]
pub fn test() -> Vec<u8> {
    b"Hello World!".to_vec()
}
