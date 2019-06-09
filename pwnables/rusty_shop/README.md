## Summary
 - category: pwnables
 - name: rusty_shop
 - description: Our Rust dev doesn't like updating his compiler very often, he says it's too much hassle.
 - difficulty: easy-medium
 - flag: See "flag" file


## Vulnerability
 - There is an integer overflow in the Rust function str::repeat (CVE-2018-1000810)
 - Players can specify a large quantity of an item to add to their basket. When checking out this will overflow the heap with the item name
 - The detect_hacking function is being called repeatedly in a different thread, this function uses the vtables of some objects
 - Players can overflow one of the vtable pointers on the heap and point it to FUNC-0x18 which will then be called by detect_hacking 

## Build
 - Install rustc <= 1.29.0
 - Run `cargo build`
 - Binary is in ./target/debug/rusty_shop
 - For solve.py - find &FUNC in the binary with `nm -C target/debug/rusty_shop | grep FUNC`
