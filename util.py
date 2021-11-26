import numpy as np


def parse_text(text_file):
    with open(text_file) as fh:
        text = fh.read()
    text_str_list = text.strip().split("\n")
    points = []
    for text_str in text_str_list:
        text_str = text_str.strip()
        values = text_str.split(" ")
        points.append([float(value) for value in values if not value == ""])
    return np.array(points)
