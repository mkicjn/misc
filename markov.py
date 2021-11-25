#!/usr/bin/python3
import sys
import random

(_, output_size, memory) = sys.argv

output_size = int(output_size)
memory = int(memory)

# Build a list of input syms
input_syms = sys.stdin.read()

# Construct the vocabulary and weights
history = (None,) * memory
vocab = {}

for symbol in input_syms:
    if history not in vocab:
        vocab[history] = {}
    if symbol not in vocab[history]:
        vocab[history][symbol] = 0
    vocab[history][symbol] += 1
    history = history[1:] + (symbol,)

# Generate a random string according to the vocabulary's weights
history = random.choice(list(vocab.keys()))
for i in range(output_size):
    try:
        symbol = random.choices(list(vocab[history].keys()),
                        weights=list(vocab[history].values()))[0]
    except:
        print('\n')
        history = random.choice(list(vocab.keys()))
        continue
    print(symbol, end='')
    history = history[1:] + (symbol,)
print()
