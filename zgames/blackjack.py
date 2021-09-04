import random


def count_cards(cards):
    total = 0
    for card in cards:
        if card.isnumeric():
            total += int(card)
        elif card == 'A':
            if total > 10:
                total += 1
            else:
                total += 11
        else:
            total += 10
    return total


suit = ['A'] + [str(n) for n in range(2, 11)] + ['J', 'Q', 'K']
deck = suit * 4

random.shuffle(deck)

dealer_cards = []
dealer_cards.append(deck.pop(0))
dealer_cards.append(deck.pop(0))

print("Dealer face-up card:", dealer_cards[0])

player_cards = []
player_cards.append(deck.pop(0))
player_cards.append(deck.pop(0))

while True:
    # 1: Player assesses his cards' value
    player_total = count_cards(player_cards)
    # 2: Player sees their cards
    print("Your cards:", ", ".join(player_cards))
    # 3: Stop if the player can't hit anymore
    if player_total >= 21:
        break
    # 4: Player makes a move
    user_response = input("Hit or stand? ")
    if user_response[0] == 'h' or user_response[0] == 'H':
        # Hit
        player_cards.append(deck.pop(0))
    elif user_response[0] == 's' or user_response[0] == 'S':
        # Stand
        break

if player_total > 21:
    print("Bust.")
    exit()

while True:
    print("Dealer's cards:", ", ".join(dealer_cards))
    dealer_total = count_cards(dealer_cards)
    if dealer_total >= 17:
        break
    dealer_cards.append(deck.pop(0))

if dealer_total > 21:
    print("Dealer bust. You win!")
elif player_total > dealer_total:
    print("You win!")
elif dealer_total > player_total:
    print("You lose!")
else:
    print("You tie...")