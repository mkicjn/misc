#!/usr/bin/python3
import sys
import re

DEBUG = True

# Read objdump command from stdin
functions = {}
cur_name = None
for line in sys.stdin.readlines():
    line = line.rstrip()
    if line == '':
        # Empty line => function over
        cur_name = None
    elif cur_name is not None:
        # Mid-function => process line and append to current function
        if cur_name not in functions:
            functions[cur_name] = []
        line = line.split('\t')
        line[0] = line[0].lstrip().rstrip(':')
        line[1] = ''.join(line[1].split())
        if len(line) >= 3:
            # Full instruction
            line[2] = '\t'.join(line[2:])
            functions[cur_name].append(line)
        else:
            # Last instruction, continued
            functions[cur_name][-1][1] += line[1]
    else:
        # Not mid-function => look for next function name
        matches = re.search('<(\w+)>:', line)
        if matches:
            cur_name = matches[1]


# How to print the results
def make_wide(content, width):
    return f'{{:<{width}}}'.format(content)

def format_inst(inst):
    COL_WIDTHS = [8, 24, 40]
    return ''.join([make_wide(c, w) for c, w in zip(inst, COL_WIDTHS)])

def format_function(name, insts):
    return f'{name}:\n' + '\n'.join([format_inst(inst) for inst in insts])

def format_functions(functions):
    return '\n\n'.join([format_function(name, func) for name, func in functions.items()])

def show_functions():
    print(format_functions(functions))


# Infer return instruction from `bye`
ret_inst = functions['bye_code'][-1]
print(f'Inferred return instruction: {ret_inst}')

# Delete functions which we can't or don't want to process further
del functions['next_code']
del functions['bye_code']
del functions['jmp_code']
del functions['jz_code']

# Strip the inner interpreter from each remaining word
for name, insts in functions.items():
    if "<next_code>" in insts[-1][2]:
        del insts[-1]
    else:
        message = f'Cannot yet handle {name}, which does not branch to the inner interpreter:\n'
        message += format_function(name, insts)
        raise NotImplementedError(message)

if DEBUG:
    show_functions()

# TODO:
# * Infer encoding of call/jmp/jne
