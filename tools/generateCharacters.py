import sys
import re

with open('contrast.flf') as f:
    header = f.readline()
    if match := re.search(r'flf2a\$ (-?\d+) (-?\d+) (-?\d+) (-?\d+) (-?\d+)', header):
        height, maxLen, comments = int(match[1]), int(match[3]), int(match[5])
    else:
        sys.exit(0)

    # skip comments
    for i in range(comments):
        f.readline()

    chars = []
    for i in range( ord('}') - ord(' ') ):
        data = ""
        width = 0
        for row in range(height):
            line = f.readline().replace('@', '').replace('\r', '').replace('\n', '') \
                .replace('.', '0') \
                .replace('%', '1')
            width = len(line) # Should be equal over lines
            data += line
        chars.append( (width, data) )

text = "".join([c[1] for c in chars])
print( f'const char _text[] = "{text}";\n')

print( 'const Character characters[] = {')
offset = 0
for i, c in enumerate(chars):
    width = c[0]
    i = i + ord(' ')
    print( f'\tCharacter({width}, {height}, &_text[{offset}]), /* {chr(i)} */' )
    offset += width * height
print( '};')