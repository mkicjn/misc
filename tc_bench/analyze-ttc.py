#!/usr/bin/python3
import sys

DEBUG=True

segments = [[]]
i = 0

# Read lines into segments based on ending in an instruction targeting tt_interp
for line in sys.stdin.readlines():
    segments[i].append(line.lstrip().rstrip().split('\t'))
    if "<tt_interp+" in line:
        i += 1
        segments.append([])

# Assume the last instruction in the last segment is a return instruction
ret_inst = segments[-1][-1][-1]
print(f'Assuming return instruction is `{ret_inst}`')

# Cut first and last segments out to isolate instructions
segments = segments[2:-1]
print(f'{len(segments)} segments detected')

if DEBUG:
    for line in segments[1:-1]:
        for segment in line:
            print(segment)
        print('')

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
    for line in segments:
        for segment in line:
            print(segment)
        print('')

# Flatten the list to get all instructions
insts = [inst for segment in segments for inst in segment]

# Expand segments not branching to the most common branch target
def find_destination(address):
    key = f"{address}:"
    for segment in segments:
        hits = [i for (i, line) in enumerate(segment) if line[0] == key]
        if hits:
            return segment[hits[0]:]
    return None

for segment in segments:
    if segment[-1][-1] != common_target:
        last_jmp = segment[-1]
        target = last_jmp[-1].split()[1]
        destination = find_destination(target)
        if destination is None:
            print(f'Target {target} not found in other primitives; trying that as the common target')
            common_target = last_jmp[-1]

for (i, segment) in enumerate(segments):
    if segment[-1][-1] != common_target:
        last_jmp = segment[-1]
        target = last_jmp[-1].split()[1]
        destination = find_destination(target)
        if destination:
            segments[i] = segment[:-1] + destination
        else:
            raise RuntimeError(f'Target {target} not found in other primitives')

if DEBUG:
    print('Post branch target expansion:')
    for line in segments:
        for segment in line:
            print(segment)
        print('')

# TODO
# Infer the encoding of branching instructions (relative / absolute)
# (Meta: Test on aarch64-linux-gnu toolchain)
