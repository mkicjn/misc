#!/usr/bin/python3
import sys

DEBUG=True

segments = [[]]

def format_inst(inst):
    COL_WIDTHS = [8, 20, 40]
    line = ""
    for field, width in zip(inst, COL_WIDTHS):
        line += f'{{:<{width}}}'.format(field)
    return line

def show_segments():
    global segments
    for segment in segments:
        for inst in segment:
            print(format_inst(inst))
        print('')


# Read lines into segments based on ending in an instruction targeting tt_interp
for line in sys.stdin.readlines():
    fields = line.lstrip().rstrip().split('\t')
    # Postprocessing and insertion
    try:
        fields[0] = fields[0].rstrip(':')
        fields[1] = ''.join(fields[1].split(' '))
        fields = fields[:2] + [''.join(fields[2:])]
    except IndexError:
        continue
    segments[-1].append(fields)
    # Internal branch detection
    if "<tt_interp+" in line:
        segments.append([])


# Assume the last instruction in the last segment is a return instruction
ret_inst = segments[-1][-1][-1]
print(f'Assuming return instruction is `{ret_inst}`')

# Cut first and last segments out to isolate instructions
segments = segments[2:-1]
print(f'{len(segments)} segments detected')

if DEBUG:
    show_segments()


# Find how many times each branch target in tt_interp is targeted
target_hits = {}
for jmp_line in [segment[-1] for segment in segments]:
    target = jmp_line[-1]
    if "tt_interp" not in target:
        continue
    if target not in target_hits:
        target_hits[target] = 1
    else:
        target_hits[target] += 1

if DEBUG:
    print('Hits per interior branch:')
    for (key, value) in target_hits.items():
        print(f'{key}\t{value}')


# Find the most common branching instruction targeting somewhere inside tt_interp
common_target = max(target_hits, key=target_hits.get)
print('Most common interior branching instruction:')
print(common_target)


# Assume this is an unconditional branch instruction
jmp_inst = common_target.split(' ')[0]
print(f'Assuming unconditional branch is `{jmp_inst}`')


# Find all unconditional branch instructions in each segments
segment_jmps = []
for segment in segments:
    jmp_indices = [i for (i, line) in enumerate(segment) if jmp_inst in line[-1]]
    segment_jmps.append(jmp_indices)

if DEBUG:
    print('Jump instruction indices:')
    for jmps in segment_jmps:
        print(jmps)


# If a segment has more than 1 unconditional branch, delete from the beginning up through the penultimate jump
for (i, segment) in enumerate(segments):
    if len(segment_jmps[i]) > 1:
        penultimate_jmp = segment_jmps[i][-2]
        segments[i] = segments[i][penultimate_jmp+1:]

if DEBUG:
    print('Post penultimate branch removal:')
    show_segments()


# Flatten the list to get all instructions
insts = [inst for segment in segments for inst in segment]


# Expand segments not branching to the most common branch target
def find_destination(address):
    for segment in segments:
        hits = [i for (i, line) in enumerate(segment) if line[0] == address]
        if hits:
            return segment[hits[0]:]
    return None

for segment in segments:
    if segment[-1][-1] != common_target:
        last_jmp = segment[-1]
        (op, target, *rest) = last_jmp[-1].split()
        destination = find_destination(target)
        if destination is None:
            print(f'Target {target} not found in other primitives; trying that as the common target')
            common_target = last_jmp[-1]

unrecognized = []
for (i, segment) in enumerate(segments):
    if segment[-1][-1] != common_target:
        last_jmp = segment[-1]
        (op, target, *rest) = last_jmp[-1].split()
        if op != jmp_inst: # 
            unrecognized.append(op) #
        destination = find_destination(target)
        if destination:
            segments[i] = segment[:-1] + destination
        else:
            raise RuntimeError(f'Target {target} not found in other primitives')

if DEBUG:
    print('Post branch target expansion:')
    show_segments()

if unrecognized:
    unrecognized = ', '.join(unrecognized)
    raise NotImplementedError(f"Not sure how to handle {unrecognized} yet")


# TODO
# Infer the encoding of branching instructions (relative / absolute)
# (Meta: Test on aarch64-linux-gnu toolchain)
