/// Encodes a 32-bytes integer into big-endian bytes.
#let big-endian-encode(value) = {
  bytes((
    calc.rem(calc.quo(value, 0x1000000), 0x100),
    calc.rem(calc.quo(value, 0x10000), 0x100),
    calc.rem(calc.quo(value, 0x100), 0x100),
    calc.rem(calc.quo(value, 0x1), 0x100),
  ))
}

/// Decodes a big-endian integer from the given bytes.
#let big-endian-decode(bytes) = {
  let result = 0
  for byte in array(bytes) {
    result = result * 256
    result = result + byte
  }
  return result
}

#let encode-str-array(arr) = {
  let encoded-len = big-endian-encode(arr.len())
  let encoded-parts = (encoded-len,) + arr.map(str => bytes(array(bytes(str)) + (0,)))
  bytes(encoded-parts.map(array).fold((), (acc, x) => acc + x))
}

/// Transforms bytes into an array whose elements are all `bytes` with the specified length.
#let group-bytes(buffer, group-len) = {
  assert(calc.rem(buffer.len(), group-len) == 0)
  array(buffer).fold((), (acc, x) => {
    if acc.len() != 0 and acc.last().len() < group-len {
      acc.last().push(x)
      acc
    } else {
      acc + ((x,),)
    }
  }).map(bytes)
}

/// Group elements of the array in pairs.
#let array-to-pairs(arr) = {
  assert(calc.even(arr.len()))
  arr.fold((), (acc, x) => {
    if acc.len() != 0 and acc.last().len() < 2 {
      acc.last().push(x)
      acc
    } else {
      acc + ((x,),)
    }
  })
}
