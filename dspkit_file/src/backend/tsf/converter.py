# Simple script to convert soundfont and midi files to c-compatible arrays

from os import listdir
from textwrap import fill


def file2c(fn, array_name):
    """ Read a file and return a C-compatible array with its binary data """

    # Open file and generate a list with hex-encoded byte contents
    with open(fn, 'rb') as f:
        rawdata = f.read()
    hexdata = [hex(x) for x in rawdata]

    # Generate C-compatible array

    # Header
    c_str = 'const char ' + array_name + '[' + str(len(hexdata)) + '] = {'

    # Data contents
    for x in hexdata:
        c_str += x + ', '

    # Remove last ', ' elements
    c_str = c_str[:-2]

    # Tail
    c_str += '};'
    return c_str


def files2c(readdir, ext):
    """ Read all files in a given directory with a given extension and convert them to C-compatible arrays """
    names = listdir(readdir)

    # Generate list of files matching target extension
    files = []
    for name in names:
        if ext == name.split('.')[-1]:
            files.append(name)

    # Generate all C-arrays
    c_strs = []
    for file in files:
        c_strs.append(file2c(readdir + '/' + file, file.split('.')[0]))

    return c_strs, files


def files2ptrs(dir, ext, ptr_name):
    """ Returns files2c, with additional array pointing to each output from files2c """
    # Generate all C-arrays
    c_strs, names = files2c(dir, ext)

    # Generate array of pointers to C-arrays
    ptr_arr = 'const char * ' + ptr_name + '[' + str(len(c_strs)) + '] = {'
    for name in names:
        ptr_arr += name.split('.')[0] + ', '
    ptr_arr = ptr_arr[:-2]
    ptr_arr += '};'

    c_strs.append(ptr_arr)

    return c_strs

# Convert all .mid files in an ./input directory to binary C-arrays
# Also convert an .sf2 file to a binary C-array
# Write output to tsf_data.h/.c

# Read relevant data from files

tracks = files2ptrs('./input', 'mid', 'tsf_tracks')
soundfont = file2c('./input/GXSCC_gm_033.sf2', 'tsf_soundfont')

# Generate output data for tsf_data.h
h_hdr =  '/** @file Contains all data needed by tinySoundFont library for playback */\n'
h_hdr += '#ifndef TSF_DATA_H_\n'
h_hdr += '#define TSF_DATA_H_\n'
h_hdr += "#include <stdint.h>\n"

h_ftr = '#endif /* TSF_DATA_H_ */\n'

c_hdr = '#include "tsf_data.h"\n'

with open('tsf_data.h', 'w') as f:
    f.write(h_hdr)
    f.write(soundfont.split('=')[0].rstrip() + ';\n')
    f.write(tracks[-1].split('=')[0].rstrip() + ';\n')
    f.write('const uint32_t tsf_track_len[' + str(len(tracks)-1) + '];\n')
    f.write(h_ftr)

with open('tsf_data.c', 'w') as f:
    f.write(c_hdr)
    f.write(fill(soundfont, width=120, break_long_words=False, break_on_hyphens=False) + '\n')
    for line in tracks:
        f.write(fill(line, width=120, break_long_words=False, break_on_hyphens=False) + '\n')

    len_str = 'const uint32_t tsf_track_len[' + str(len(tracks)-1) + '] = {'

    for line in tracks[0:-1]:
        foo = line.split('[', maxsplit=1)[1]
        foo = foo.split(']', maxsplit=1)[0]
        len_str += foo + ', '

    len_str = len_str[:-2]
    len_str += '};'

    f.write(len_str)

