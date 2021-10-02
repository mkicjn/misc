#!/usr/bin/python3

LIGHT = 1
EMPTY = 0
DARK = -1

def new_board():
    board = [[EMPTY for i in range(8)] for i in range(8)]
    board[3][3] = LIGHT
    board[3][4] = DARK
    board[4][3] = DARK
    board[4][4] = LIGHT
    return board



def clear_screen():
    print("\033[2J\033[1;1H", end='')

def green_bg(string):
    return "\033[42m" + string + "\033[49m"

def white_fg(string):
    return "\033[97m" + string + "\033[39m"

def black_fg(string):
    return "\033[30m" + string + "\033[39m"


def piece_to_str(piece):
    if piece == LIGHT:
        return green_bg(white_fg("()"))
    elif piece == DARK:
        return green_bg(black_fg("()"))
    else:
        return green_bg("  ")

def pretty_board(board):
    strings = [[piece_to_str(x) for x in row] for row in board]
    return '\n'.join(map(''.join, strings))

def add_coordinates(string):
    lines = string.split('\n')
    lines = [''.join([chr(65+i)*2 for i in range(8)])] + lines
    lines[0] = ' ' + lines[0]
    for i in range(1, 9):
        lines[i] = str(i) + lines[i]
    return '\n'.join(lines)



def other_player(player):
    if player == LIGHT:
        return DARK
    elif player == DARK:
        return LIGHT
    else:
        raise ValueError("Invalid player type")

def in_bounds(y, x):
    return x >= 0 and x < 8 and y >= 0 and y < 8

def capture_direction(board, location, direction, trial=True):
    global turn
    y, x = location
    dy, dx = direction
    total = 0
    (y, x) = (y+dy, x+dx)
    while in_bounds(y, x):
        if board[y][x] == other_player(turn):
            total += 1
            if not trial:
                board[y][x] = turn
        else:
            break
        (y, x) = (y+dy, x+dx)
    if in_bounds(y, x) and board[y][x] == turn:
        return total
    else:
        return 0

def place_piece(board, location):
    global turn
    y, x = location
    if board[y][x] != EMPTY:
        return False
    # See which directions can capture pieces
    dirs = [(0, 1), (0, -1), (1, 0), (1, 1), (1, -1), (-1, 0), (-1, 1), (-1, -1)]
    capturable = [capture_direction(board, location, d, trial=True) for d in dirs]
    dirs = [t[0] for t in zip(dirs, capturable) if t[1] > 0]
    # Make move, capture, and swap turns if possible
    if len(dirs) < 1:
        return False
    for (dy, dx) in dirs:
        capture_direction(board, location, (dy, dx), trial=False)
    board[y][x] = turn
    turn = other_player(turn)
    return True

board = new_board()
turn = DARK

while True:
    clear_screen()
    print(add_coordinates(pretty_board(board)))
    try:
        move = input()
    except KeyboardInterrupt:
        break
    letter = move[0]
    number = move[1]
    x = ord(letter)-65
    y = int(number)-1
    place_piece(board, (y, x))
