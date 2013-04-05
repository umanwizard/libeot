def get_val(s, last_val):
    if s == 'N/A':
        return 0
    if s == '+':
        return 1
    if s == '-':
        return -1;
    if s == '':
        return last_val
    return int(s)
def gen_struct(row, last):
    return '{' + ', '.join(['%s' % get_val(x.strip(), y) for (x,y) in zip(row, last)]) + '}'
def gen_all(rows):
    last = None
    structs = []
    for row in rows:
        if not last:
            last = [0 for x in row]
        structs.append(gen_struct(row, last))
        last = [get_val(x.strip(), y) for (x, y) in zip(row, last)]
    return '{' + ',\n'.join(structs) + '}'

import csv
with open('file.csv', 'r') as csvfile:
    rdr = csv.reader(csvfile)
    print gen_all(rdr)
