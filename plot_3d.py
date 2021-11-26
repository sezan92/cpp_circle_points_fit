import argparse
import os

import matplotlib.pyplot as plt
import numpy as np

from util import parse_text


def plot_3d(points, filename, output_dir=None):
    fig = plt.figure()
    ax = fig.add_subplot(projection="3d")
    x = points[:, 0]
    y = points[:, 1]
    z = points[:, 2]
    ax.scatter(x, y, z)
    ax.set_xlabel("X Label")
    ax.set_ylabel("Y Label")
    ax.set_zlabel("Z Label")
    ax.set_title(filename)
    if output_dir:
        fig.savefig(os.path.join(output_dir, filename[:-3] + "png"), dpi=720)
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=str, help="text file with the 3D points.")
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="directory for saving the plots. Default:None",
    )
    args = parser.parse_args()

    points = parse_text(args.input)
    os.makedirs(args.output, exist_ok=True)
    plot_3d(points, os.path.basename(args.input), args.output)
