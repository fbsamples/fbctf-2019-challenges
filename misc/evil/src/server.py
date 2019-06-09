#! /usr/bin/env python3
import random
import select
import sys

L_WORDS = [
    "labels",
    "laboratories",
    "labs",
    "labyrinths",
    "ladders",
    "ladles",
    "lagers",
    "lakes",
    "lamebrains",
    "lampreys",
    "landlines",
    "lanyards",
    "lasagnas",
    "lathes",
    "laughingstocks",
    "launderers",
    "lawbreakers",
    "layovers",
    "laypersons",
    "leeches",
    "legumes",
    "lemurs",
    "lentils",
    "leopards",
    "leotards",
    "leprechauns",
    "lettuces",
    "ligaments",
    "limeades",
    "limericks",
    "limes",
    "lions",
    "litterbugs",
    "lizards",
    "llamas",
    "lobsters",
    "locusts",
    "loganberries",
    "logarithms",
    "lolcats",
    "lollipops",
    "loudmouths",
    "lozenges",
    "lumps",
    "luxurious",
    "lynxes",
]

FLAG = "fb{th4t5_th3_3vi1est_th1ng_!_c4n_im4g1ne}"
TIMEOUT = 5

if __name__ == '__main__':
    if 'bad' in sys.argv:
        print("Clearly you're not evil enough.")
        sys.stdout.flush()
        sys.exit(1)
    else:
        word = random.choice(L_WORDS)
        print("So you know the evil handshake?\nWhat's the secret password then?\nThe word of the day is {}.".format(word))
        sys.stdout.flush()
        i, o, e = select.select([sys.stdin], [], [], TIMEOUT)
        if i:
            x = sys.stdin.readline().strip()
        else:
            print("Too slow, move along.")
            sys.exit(2)

        #  sys.stderr.write("Received: {}\n".format(x))
        if x == "Every Villain Is {}".format(word.capitalize()):
            print(FLAG)
            sys.stdout.flush()
            sys.exit(0)
        else:
            print("Wrong password! Get lost do-gooder!")
            sys.stdout.flush()
            sys.exit(3)
