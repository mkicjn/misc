#!/usr/bin/python3
import sys
import re
import random

# This script is optimized for English text, but it can be tweaked to work on other things.

gen_count = 128
if len(sys.argv) > 1:
    gen_count = int(sys.argv[1])

memory = 1
if len(sys.argv) > 2:
    memory = int(sys.argv[2])

# Build a list of input words
input_words = re.findall(r'[\w]+\s?|[^\w]\s?', re.sub(r'\s+', ' ', sys.stdin.read()))

# Construct the vocabulary and weights
last_words = (None,) * memory
vocab = {}

for word in input_words:
    if last_words not in vocab:
        vocab[last_words] = {}
    if word not in vocab[last_words]:
        vocab[last_words][word] = 0
    vocab[last_words][word] += 1
    last_words = last_words[1:] + (word,)
# TODO: Try tracking for multiple previous words

# Generate random strings according to the vocabulary's weights
last_words = random.choice(list(vocab.keys()))
for i in range(0, gen_count):
    try:
        word = random.choices(list(vocab[last_words].keys()),
                      weights=list(vocab[last_words].values()), k=1)[0]
    except:
        print('\n')
        last_words = random.choice(list(vocab.keys()))
        continue
    #if re.match('\w+', word) is not None:
    #    print(' ', end='')
    print(word, end='')
    last_words = last_words[1:] + (word,)
print()
