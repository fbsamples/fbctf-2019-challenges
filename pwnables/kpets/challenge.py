#!/usr/bin/python
import struct
import os
import sys
import tempfile
import subprocess

useragent = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.131 Safari/537.36'

def main():
    try:
        p = subprocess.Popen('./pow.py ask 3'.split())
        p.communicate()
        if p.returncode != 2:
            exit(1)
    except:
        exit(1)

    print('Would you like to add a file to the VM? This isn\'t part of the challenge (Y/n)')    
    choice = raw_input()
    if choice.strip() != 'n':
        print('File URL (max size 1MB): ')
        url = raw_input().strip()
        tmp_file = tempfile.mktemp()

        # Do some basic validation of the URL
        if not (url.startswith('http://') or url.startswith('https://')) \
                or 'localhost' in url \
                or '::1' in url \
                or '127.0.0.1' in url:
            print('Invalid URL')
            exit(1)


        # Fetch the file
        p = subprocess.Popen(['curl', '-A', useragent, '--max-filesize', '1048576', '-o', tmp_file, url]) # max 1MB
        p.communicate()
 
        if p.returncode != 0:
            print('exited with code {}'.format(ret))
            exit(1)

        # Validate magic of the downloaded file
        with open(tmp_file) as f:
            if f.read(4) != '\x7fELF':
                #print('ELF files only')
                exit(1)

        # Make copy of initramfs and insert exploit file
        new_ramfs = tempfile.mkdtemp()
        #print('New initramfs: {}'.format(new_ramfs))
        os.system('cp -r base_qemu/initramfs/ {}'.format(new_ramfs))
        out_file = '{}/initramfs/bin/exploit'.format(new_ramfs)
        #print('Moving {} to {}'.format(tmp_file, out_file))
        os.system('mv {} {}'.format(tmp_file, out_file))

        print('Your binary is at /bin/exploit')

        # Pack new initramfs
        os.system('./pack_initramfs.sh {}/initramfs/ src/kpets.ko'.format(new_ramfs))
        os.system('./start_qemu.sh qemu/bzImage {}/initramfs.cpio'.format(new_ramfs))
        os.system('rm -r {}'.format(new_ramfs))
    else:
        # Use standard initramfs
        os.system('./start_qemu.sh qemu/bzImage qemu/initramfs.cpio')

if __name__=="__main__":
    main()
