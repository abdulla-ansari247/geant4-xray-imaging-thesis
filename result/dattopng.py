import numpy as np
import matplotlib.pyplot as plt
import os

def load_xray_data(filename, nx=80, ny=80):
    data = np.loadtxt(filename)
    return data.reshape((ny, nx))

def save_as_png(image, outname="xray.png"):
    img = np.max(image) - image if np.max(image) > 0 else image
    plt.imshow(img, cmap="gray_r", origin="lower")
    plt.colorbar(label="Normalized deposited energy")
    plt.title("Simulated X-ray Image")
    plt.savefig(outname, dpi=300, bbox_inches="tight")
    plt.close()

if __name__ == "__main__":
    script_dir = os.path.dirname(__file__)
    filename = os.path.join(script_dir, "xray_image.dat")
    outfile = os.path.join(script_dir, "xray.png")

    data = load_xray_data(filename, nx=80, ny=80)
    save_as_png(data, outfile)
    print(f"Saved X-ray image as {outfile}")
