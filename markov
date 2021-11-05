#!/usr/bin/python3
import sys
import re
import random

# This script is optimized for English text, but it can be tweaked to work on other things.

gen_count = 128
if len(sys.argv) > 1:
    gen_count = int(sys.argv[1])

last_word = None
vocab = {last_word: {}}

# Build a list of input words
input_words = re.findall(r"[\w]+|[\.!?;]", sys.stdin.read())

# Construct the vocabulary (including weights)
for word in input_words:
    if word not in vocab:
        vocab[word] = {}
    if word not in vocab[last_word]:
        vocab[last_word][word] = 0
    vocab[last_word][word] += 1
    last_word = word
# TODO: Try tracking for multiple previous words

# Generate random strings according to the vocabulary's weights
last_word = None
for i in range(0, gen_count):
    try:
        word = random.choices(list(vocab[last_word].keys()),
                      weights=list(vocab[last_word].values()), k=1)[0]
    except IndexError:
        word = None
        continue
    if re.match('\w+', word) is not None:
        print(' ', end='')
    print(word, end='')
    last_word = word
print()
