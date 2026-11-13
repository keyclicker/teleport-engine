#!/usr/bin/env python3
"""Draw the demo game's sprites into Resources/Sprites/.

Original pixel art, drawn with code so it's easy to tweak. Needs Pillow:
`python3 Tools/make_sprites.py`. Transparent pixels have alpha 0.
"""

import math
import pathlib

from PIL import Image, ImageDraw

OUT = pathlib.Path(__file__).resolve().parent.parent / "Resources" / "Sprites"

# ---- Palette -------------------------------------------------------------

OUTLINE = (30, 12, 8, 255)
SKIN = (150, 52, 34, 255)
SKIN_DARK = (104, 34, 22, 255)
SKIN_LIGHT = (196, 86, 52, 255)
HORN = (226, 210, 170, 255)
EYE = (255, 230, 60, 255)
MOUTH = (60, 8, 8, 255)
TOOTH = (240, 240, 220, 255)
FIRE = [(255, 250, 200, 255), (255, 210, 60, 255), (255, 120, 20, 255),
        (200, 40, 10, 255)]
METAL = (110, 116, 124, 255)
METAL_DARK = (60, 64, 72, 255)
METAL_LIGHT = (170, 176, 184, 255)
HAND = (214, 160, 120, 255)
HAND_DARK = (160, 108, 76, 255)
WOOD = (120, 72, 36, 255)


def canvas(w, h):
    return Image.new("RGBA", (w, h), (0, 0, 0, 0))


def save(img, name):
    OUT.mkdir(parents=True, exist_ok=True)
    img.save(OUT / name)


# ---- Monster -------------------------------------------------------------

def monster(stride=0, attack=False, pain=False):
    """A horned demon, 48x64; stride -1/0/1 swings the legs."""
    img = canvas(48, 64)
    d = ImageDraw.Draw(img)

    # Legs
    for side, swing in ((-1, stride), (1, -stride)):
        x = 24 + side * 7 + swing * 3
        d.rectangle([x - 4, 44, x + 4, 61], fill=SKIN_DARK, outline=OUTLINE)
        d.rectangle([x - 5, 59, x + 5, 63], fill=OUTLINE)

    # Body
    d.ellipse([10, 22, 38, 50], fill=SKIN, outline=OUTLINE)
    d.ellipse([16, 28, 32, 44], fill=SKIN_LIGHT)
    for y in (32, 37, 42):
        d.line([19, y, 29, y], fill=SKIN_DARK)

    # Arms: down, or raised with a fireball between the claws
    if attack:
        for side in (-1, 1):
            d.polygon([(24 + side * 12, 30), (24 + side * 18, 28),
                       (24 + side * 10, 6), (24 + side * 5, 8)],
                      fill=SKIN, outline=OUTLINE)
        for r, c in zip((7, 5, 3, 1), reversed(FIRE)):
            d.ellipse([24 - r, 5 - r, 24 + r, 5 + r], fill=c)
    else:
        for side in (-1, 1):
            d.polygon([(24 + side * 12, 28), (24 + side * 19, 30),
                       (24 + side * 18, 46), (24 + side * 13, 46)],
                      fill=SKIN, outline=OUTLINE)
            for i in range(3):
                x = 24 + side * (13 + i * 2)
                d.line([x, 46, x, 50], fill=HORN)

    # Head and horns
    head_y = 14 if not attack else 16
    for side in (-1, 1):
        d.polygon([(24 + side * 6, head_y - 4), (24 + side * 14, head_y - 14),
                   (24 + side * 10, head_y - 2)], fill=HORN, outline=OUTLINE)
    d.ellipse([14, head_y - 8, 34, head_y + 12], fill=SKIN, outline=OUTLINE)

    if pain:
        # Squeezed eyes, open mouth
        for side in (-1, 1):
            d.line([24 + side * 3, head_y, 24 + side * 7, head_y - 1],
                   fill=OUTLINE, width=2)
        d.ellipse([20, head_y + 4, 28, head_y + 10], fill=MOUTH)
    else:
        for side in (-1, 1):
            d.ellipse([24 + side * 5 - 2, head_y - 3, 24 + side * 5 + 2,
                       head_y + 1], fill=EYE)
        d.rectangle([18, head_y + 5, 30, head_y + 8], fill=MOUTH)
        for x in (19, 22, 25, 28):
            d.polygon([(x, head_y + 5), (x + 2, head_y + 5),
                       (x + 1, head_y + 7)], fill=TOOTH)

    return img


def monster_dead():
    """The demon in a heap, 48x64 with the heap at the bottom."""
    img = canvas(48, 64)
    d = ImageDraw.Draw(img)

    d.ellipse([4, 50, 44, 63], fill=(110, 10, 10, 255))
    d.ellipse([8, 46, 40, 60], fill=SKIN_DARK, outline=OUTLINE)
    d.ellipse([28, 44, 42, 56], fill=SKIN, outline=OUTLINE)
    d.polygon([(38, 46), (46, 40), (40, 49)], fill=HORN, outline=OUTLINE)
    d.line([30, 49, 34, 50], fill=OUTLINE, width=2)
    d.line([12, 52, 20, 50], fill=SKIN_LIGHT, width=2)

    return img


# ---- Projectiles and effects --------------------------------------------

def fireball(phase):
    img = canvas(16, 16)
    d = ImageDraw.Draw(img)

    for r, c in zip((8, 6, 4, 2), reversed(FIRE)):
        d.ellipse([8 - r, 8 - r, 7 + r, 7 + r], fill=c)

    # Sparks that turn with the phase
    for i in range(4):
        a = phase * math.pi / 4 + i * math.pi / 2
        x, y = 8 + 7 * math.cos(a), 8 + 7 * math.sin(a)
        d.point([x, y], fill=FIRE[0])

    return img


def puff(size):
    """Explosion or bullet puff, `size` from 1 (small) to 3."""
    img = canvas(24, 24)
    d = ImageDraw.Draw(img)

    r = 4 + size * 3
    for k, c in enumerate(reversed(FIRE)):
        rr = r - k * 2
        if rr > 0:
            d.ellipse([12 - rr, 12 - rr, 11 + rr, 11 + rr], fill=c)

    return img


# ---- Pickups -------------------------------------------------------------

def medkit():
    img = canvas(24, 16)
    d = ImageDraw.Draw(img)

    d.rectangle([1, 3, 22, 15], fill=(230, 230, 230, 255), outline=OUTLINE)
    d.rectangle([9, 5, 14, 13], fill=(200, 20, 20, 255))
    d.rectangle([6, 7, 17, 11], fill=(200, 20, 20, 255))
    d.rectangle([8, 0, 15, 3], fill=METAL_DARK)

    return img


def ammo_box():
    img = canvas(20, 14)
    d = ImageDraw.Draw(img)

    d.rectangle([0, 4, 19, 13], fill=(80, 90, 40, 255), outline=OUTLINE)
    for x in range(2, 18, 4):
        d.rectangle([x, 0, x + 2, 5], fill=(200, 160, 40, 255),
                    outline=OUTLINE)

    return img


def shotgun_pickup():
    img = canvas(40, 12)
    d = ImageDraw.Draw(img)

    d.rectangle([0, 4, 26, 7], fill=METAL, outline=OUTLINE)
    d.polygon([(24, 3), (39, 5), (39, 11), (26, 9)], fill=WOOD,
              outline=OUTLINE)
    d.rectangle([10, 7, 18, 9], fill=WOOD)

    return img


# ---- First-person weapons ------------------------------------------------

def muzzle_flash(d, x, y, r):
    for k, c in enumerate(FIRE):
        rr = r - k * r // 4
        d.polygon([(x, y - rr), (x + rr // 2, y), (x, y + rr // 3),
                   (x - rr // 2, y)], fill=c)
        d.ellipse([x - rr // 2, y - rr // 2, x + rr // 2, y + rr // 2],
                  fill=c)


def pistol(firing):
    """Held pistol seen from behind, 64x56, bottom-anchored."""
    img = canvas(64, 56)
    d = ImageDraw.Draw(img)

    if firing:
        muzzle_flash(d, 32, 12, 14)

    # Slide and frame
    d.polygon([(26, 20), (38, 20), (40, 38), (24, 38)], fill=METAL,
              outline=OUTLINE)
    d.rectangle([28, 20, 36, 23], fill=METAL_LIGHT)
    d.rectangle([30, 17, 34, 20], fill=METAL_DARK, outline=OUTLINE)

    # Hand
    d.polygon([(20, 36), (44, 36), (48, 56), (16, 56)], fill=HAND,
              outline=OUTLINE)
    d.line([24, 44, 40, 44], fill=HAND_DARK)
    d.line([22, 50, 42, 50], fill=HAND_DARK)

    return img


def shotgun(firing):
    """Held shotgun, 80x60, bottom-anchored."""
    img = canvas(80, 60)
    d = ImageDraw.Draw(img)

    if firing:
        muzzle_flash(d, 40, 10, 18)

    # Barrels
    d.polygon([(33, 16), (47, 16), (50, 44), (30, 44)], fill=METAL,
              outline=OUTLINE)
    d.line([40, 16, 40, 44], fill=METAL_DARK)
    d.rectangle([34, 16, 38, 20], fill=OUTLINE)
    d.rectangle([42, 16, 46, 20], fill=OUTLINE)

    # Pump and hands
    d.polygon([(28, 34), (52, 34), (56, 46), (24, 46)], fill=WOOD,
              outline=OUTLINE)
    d.polygon([(22, 44), (58, 44), (64, 60), (16, 60)], fill=HAND,
              outline=OUTLINE)
    d.line([24, 52, 56, 52], fill=HAND_DARK)

    return img


# ---- Main ----------------------------------------------------------------

def main():
    save(monster(-1), "monster-walk1.png")
    save(monster(1), "monster-walk2.png")
    save(monster(0, attack=True), "monster-attack.png")
    save(monster(0, pain=True), "monster-pain.png")
    save(monster_dead(), "monster-dead.png")

    save(fireball(0), "fireball1.png")
    save(fireball(1), "fireball2.png")
    for size in (1, 2, 3):
        save(puff(size), f"puff{size}.png")

    save(medkit(), "medkit.png")
    save(ammo_box(), "ammo.png")
    save(shotgun_pickup(), "shotgun-pickup.png")

    save(pistol(False), "pistol.png")
    save(pistol(True), "pistol-fire.png")
    save(shotgun(False), "shotgun.png")
    save(shotgun(True), "shotgun-fire.png")


if __name__ == "__main__":
    main()
