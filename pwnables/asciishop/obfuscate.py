#!/usr/bin/env python

import sys
import shlex
import subprocess

OFFSET = -272

functions = [
    "as_read_id",
    "as_find_chunk_by_id",
    "as_asciishop_menu",
    "as_prompt_user_for_image",
    "as_create_new_pool",
    "as_touchup_change_byte",
    "as_banner",
    "as_readn",
    "as_get_chunk_header",
    "as_find_free_chunk_in_pool",
    "as_get_chunk_ascii",
    "as_readline",
    "as_validate_ascii_header",
    "as_handle_asciishop_touchup",
    "as_deallocate_ascii_chunk_by_id",
    "as_print_ascii_grid",
    "as_find_available_pool",
    "as_setup",
    "as_handle_upload_ascii",
    "as_coordinates",
    "as_handle_delete_ascii",
    "as_handle_asciishop_filter",
    "as_allocate_chunk",
    "as_deallocate_chunk",
    "as_read_coordinates",
    "as_writen",
    "as_handle_download_ascii",
    "as_deallocate_ascii_chunk",
    "as_init_pools",
    "as_is_pool_full",
    "as_read_input",
]

data = [
    "as_io_buffer",
    "as_pool_count",
    "as_image_pools"
]

def is_in(symbol, array):
    for string in array:
        if string.startswith(symbol):
            return string 
    return None

def execute(command):
    vargs = shlex.split(command)
    p = subprocess.Popen(vargs, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    out, err = p.communicate()

    return out


if __name__ == "__main__":
    dynsym = execute("readelf --dyn-syms asciishop")
    l_dynsym = [line.strip() for line in dynsym.split("\n")]

    symbols = []
    for line in l_dynsym:
        a = line.split()
        if len(a) > 3:
            sym = a[-1]
            
            if sym.startswith("as_"):
                #print("\"%s\"," % sym)
                symbols.append((a[1], sym))

    
    with open("asciishop.c", 'r') as fp:
        asciishop = fp.read()

    for address, symbol in symbols:
        if is_in(symbol, functions):
            a = is_in(symbol, functions)
            address = "sub_" + hex(int(address.lstrip("0"), 16) + OFFSET)[2:]
            asciishop = asciishop.replace(a, address)
        elif is_in(symbol, data):
            a = is_in(symbol, data)
            address = "data_" + hex(int(address.lstrip("0"), 16) + OFFSET)
            asciishop = asciishop.replace(a, address)

    with open("asciishop_dirty.c", 'w') as fp:
        fp.write(asciishop)
