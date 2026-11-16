"""
Find the tumour hidden behind each bone by comparing the frontal shot (0 cm)
with an angled shot (default -20 cm).

Steps
-----
1. 0 cm shot: the tumours sit directly behind the bone shafts, so only the
   bones are seen. Trace the outline of every bone (x, y on the detector)
   and save it to bone_outline_0cm.csv. Two cuts are used: a tight one
   (EDGE_CUT) that sits on the real bone edge for the saved/plotted outlines,
   and a looser one (OUTLINE_CUT) that also catches the faint tumour, used
   for steps 3-6.
2. Angled shot: trace the outline of whatever is seen there. Moving the gun
   to -20 cm moves the bone shadow on the detector, and because the bone is
   6 mm thick and the rays now cross it on a slant, the outline is also a
   bit stretched (blurred) in y. Saved to outline_<shot>.csv.
3. Work out how the bone outline changed: for every detector column compare
   the lower and upper edge in both shots. The median over the columns is the
   bone's shift (and stretch). The median ignores the few columns where the
   tumour pokes out, so the tumour does not spoil this.
4. Move the 0 cm outline by that shift/stretch. That is where the bone alone
   should be in the angled shot. Anything in the angled outline that sticks
   out past it is the extra part: the tumour, seen from the side.
5. Width of the extra part: (number of columns) x pixel size. At a steep
   gun angle the tumour slides fully out from behind the bone, so this width
   is the tumour's diameter on the detector; half of it is the radius.
   (How far the bump sticks out is NOT used: the tumour's faint rim makes
   that badly measured.)
6. Undo the gun displacement with the same calculation as for the bone. A
   point at depth z moves by  shift = y_src * (1 - M(z))  on the detector when
   the gun moves from 0 to y_src, where M(z) is the magnification. The bone's
   shift gives the bone's M; the tumour sits right behind the bone, so the
   same M is used. Detector radius / M = real radius.
   Works best at the steepest shots (+-20 cm). At smaller angles part of the
   tumour is still hidden and the radius comes out too small.

Usage:  python3 analyze.py                    (0cm vs -20cm)
        python3 analyze.py --shot ../raw/xray_image_+20cm.dat
"""

import argparse
import os
import re

import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import gaussian_filter, median_filter, label

# ---------------------------------------------------------------------------
# Geometry (mm) - keep in sync with construction.cc / generator.hh
# ---------------------------------------------------------------------------
Z_SOURCE = -150.0   # source plane
Z_DET = 240.0       # detector plane
PITCH = 2.5         # pixel size
N_PIX = 160         # detector is N_PIX x N_PIX, centred on x = y = 0
X0 = -0.5 * N_PIX * PITCH + 0.5 * PITCH   # centre of pixel 0 = -198.75 mm
Z_BONE = 80.0       # bone centre plane (only used for the "expected" printout)

# Image settings
SMOOTH_SIGMA = 1.0  # Gaussian smoothing (pixels) against noise
BG_SIZE = 41        # box (pixels) for the local background estimate
BONE_CUT = 0.75     # relative intensity below this = bone core (used to find bones)
EDGE_CUT = 0.85     # bone outline (saved/plotted): halfway between background (1.0)
                    # and bone (~0.68), so it sits on the real bone edge
OUTLINE_CUT = 0.93  # tumour search: below this = "something is there". Looser than
                    # EDGE_CUT because the tumour bump is faint (~0.8-0.9)
EXTRA_TOL = 0.5     # pixels the angled outline must stick out to count as extra
MIN_EXTRA_COLS = 3  # at least this many neighbouring columns for a tumour

# Bones ordered by x
BONE_NAMES = ["left", "centre", "right"]


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def source_y_from_name(path):
    """xray_image_-20cm_test.dat -> -200.0 (source y in mm)."""
    m = re.search(r"_([+-]?\d+(?:\.\d+)?)cm(_\w+)?\.dat$", path)
    return float(m.group(1)) * 10.0


def shot_suffix(path):
    """xray_image_-20cm_test.dat -> "_test" ("" if there is no suffix)."""
    m = re.search(r"cm(_\w+)?\.dat$", path)
    return m.group(1) or ""


def pix_to_mm(p):
    """Pixel index (can be fractional) -> detector coordinate in mm."""
    return X0 + PITCH * np.asarray(p, dtype=float)


def magnification(z):
    return (Z_DET - Z_SOURCE) / (z - Z_SOURCE)


def depth_from_magnification(M):
    return Z_SOURCE + (Z_DET - Z_SOURCE) / M


def load_relative(path):
    """Load image (data[row=y, col=x]) and divide by the local background so
    1.0 = nothing in the way. Pixels outside the beam are set to 1.0."""
    data = np.loadtxt(path)
    img = gaussian_filter(data, SMOOTH_SIGMA)
    bg = median_filter(img, size=BG_SIZE)
    rel = np.ones_like(img)
    lit = bg > 0.2 * np.median(bg)
    rel[lit] = img[lit] / bg[lit]
    return rel


def find_bones(rel):
    """The three largest dark blobs that do not touch the image border
    (the border can be dark where the beam field ends), sorted left to right.
    Returns a list of (col_lo, col_hi, row_lo, row_hi)."""
    labels, _ = label(rel < BONE_CUT)
    border = np.unique(np.concatenate([labels[0], labels[-1], labels[:, 0], labels[:, -1]]))
    sizes = np.bincount(labels.ravel())
    sizes[0] = 0
    sizes[border] = 0
    biggest = np.argsort(sizes)[::-1][:len(BONE_NAMES)]
    boxes = []
    for b in biggest:
        rows, cols = np.where(labels == b)
        boxes.append((cols.min(), cols.max(), rows.min(), rows.max()))
    return sorted(boxes)


def crossing(rel, col, i_in, i_out, cut):
    """Sub-pixel row where rel crosses `cut` between rows i_in (inside)
    and i_out (outside), by linear interpolation."""
    a, b = rel[i_in, col], rel[i_out, col]
    t = (cut - a) / (b - a) if b != a else 0.5
    return i_in + t * (i_out - i_in)


def column_edges(rel, cols, row_lo, row_hi, cut, pad=15):
    """Outline of one object, column by column.

    For each column, start at the darkest pixel of the bone core and walk
    down and up while the pixel is still below `cut`. The two crossing
    points are the lower and upper edge. Anything touching the bone (like a
    tumour) is therefore included; separate noise specks are not.
    Returns arrays (low_edge, high_edge) in fractional rows.
    """
    n = rel.shape[0]
    low = np.full(len(cols), np.nan)
    high = np.full(len(cols), np.nan)
    r0, r1 = max(row_lo - pad, 0), min(row_hi + pad, n - 1)
    for k, c in enumerate(cols):
        seg = rel[r0:r1 + 1, c]
        start = r0 + int(np.argmin(seg))
        if rel[start, c] >= cut:
            continue
        i = start
        while i - 1 >= 0 and rel[i - 1, c] < cut:
            i -= 1
        low[k] = crossing(rel, c, i, i - 1, cut) if i > 0 else i
        j = start
        while j + 1 < n and rel[j + 1, c] < cut:
            j += 1
        high[k] = crossing(rel, c, j, j + 1, cut) if j < n - 1 else j
    return low, high


def longest_run(mask):
    """Indices of the longest run of True values in a 1D boolean array."""
    best, cur = [], []
    for i, m in enumerate(mask):
        if m:
            cur.append(i)
            if len(cur) > len(best):
                best = list(cur)
        else:
            cur = []
    return np.array(best, dtype=int)


def save_outline(path, outlines):
    """outlines: {name: (cols, low, high)} -> CSV with one row per edge point."""
    with open(path, "w") as f:
        f.write("bone,edge,col,row,x_mm,y_mm\n")
        for name, (cols, low, high) in outlines.items():
            for edge, rows in (("low", low), ("high", high)):
                for c, r in zip(cols, rows):
                    if np.isfinite(r):
                        f.write(f"{name},{edge},{c},{r:.3f},"
                                f"{pix_to_mm(c):.3f},{pix_to_mm(r):.3f}\n")


# ---------------------------------------------------------------------------
# Main analysis
# ---------------------------------------------------------------------------
def analyse_bone(name, rel0, rel1, box0, box1, y_src):
    c_lo, c_hi = box0[0], box0[1]
    cols = np.arange(c_lo, c_hi + 1)      # x does not change when the gun moves in y

    # 1 + 2: outlines in both shots.
    # Tight outline (EDGE_CUT): on the real bone edge, for saving and plotting.
    edge_low0, edge_high0 = column_edges(rel0, cols, box0[2], box0[3], EDGE_CUT)
    edge_low1, edge_high1 = column_edges(rel1, cols, box1[2], box1[3], EDGE_CUT)
    # Loose outline (OUTLINE_CUT): includes the faint tumour, used for steps 3-6.
    low0, high0 = column_edges(rel0, cols, box0[2], box0[3], OUTLINE_CUT)
    low1, high1 = column_edges(rel1, cols, box1[2], box1[3], OUTLINE_CUT)

    # 3: how the bone changed (median over columns ignores the tumour columns)
    d_low = np.nanmedian(low1 - low0)
    d_high = np.nanmedian(high1 - high0)
    bone_shift_px = 0.5 * (d_low + d_high)        # movement of the bone centre
    bone_stretch_px = d_high - d_low              # extra height from the slant (blur)

    # 4: where the bone alone should be in the angled shot, and the extra part
    pred_low = low0 + d_low
    pred_high = high0 + d_high
    out_low = pred_low - low1                     # > 0: sticks out below
    out_high = high1 - pred_high                  # > 0: sticks out above
    run_low = longest_run(np.nan_to_num(out_low) > EXTRA_TOL)
    run_high = longest_run(np.nan_to_num(out_high) > EXTRA_TOL)
    if len(run_low) >= len(run_high):
        side, run, edge1, pred = "below", run_low, low1, pred_low
    else:
        side, run, edge1, pred = "above", run_high, high1, pred_high

    res = dict(name=name, cols=cols, low0=low0, high0=high0, low1=low1, high1=high1,
               edge_low0=edge_low0, edge_high0=edge_high0,
               edge_low1=edge_low1, edge_high1=edge_high1,
               edge_pred_low=edge_low0 + np.nanmedian(edge_low1 - edge_low0),
               edge_pred_high=edge_high0 + np.nanmedian(edge_high1 - edge_high0),
               pred_low=pred_low, pred_high=pred_high, side=side, run=run,
               bone_shift_mm=bone_shift_px * PITCH, bone_stretch_mm=bone_stretch_px * PITCH)

    # Bone magnification and depth from its shift: shift = y_src (1 - M)
    M_bone = 1.0 - res["bone_shift_mm"] / y_src
    res["M_bone"] = M_bone
    res["z_bone"] = depth_from_magnification(M_bone)

    if len(run) < MIN_EXTRA_COLS:
        res["found"] = False
        return res
    res["found"] = True

    # 5: width of the extra part = tumour diameter on the detector
    width_det = len(run) * PITCH
    R_det = 0.5 * width_det
    x_mid = pix_to_mm(cols[run]).mean()

    # 6: undo the gun displacement / magnification with the bone's M
    res.update(width_det_mm=width_det, R_det=R_det,
               radius_mm=R_det / M_bone,
               x_tumour=x_mid / M_bone,
               x_arc=pix_to_mm(cols[run]), y_arc=pix_to_mm(edge1[run]))
    return res


def plot(results, rel0, rel1, shot_label, out_path):
    names = list(results)
    fig, axes = plt.subplots(2, len(names), figsize=(4.2 * len(names), 7.5))
    ext = [pix_to_mm(-0.5), pix_to_mm(N_PIX - 0.5)] * 2
    for j, name in enumerate(names):
        r = results[name]
        x = pix_to_mm(r["cols"])
        for i, (rel, title) in enumerate(((rel0, "0cm"), (rel1, shot_label))):
            ax = axes[i, j]
            ax.imshow(rel, cmap="gray", origin="lower", vmin=0.4, vmax=1.1, extent=ext)
            if i == 0:
                ax.plot(x, pix_to_mm(r["edge_low0"]), "c-", lw=1, label="bone outline 0cm")
                ax.plot(x, pix_to_mm(r["edge_high0"]), "c-", lw=1)
                yc = np.nanmedian(pix_to_mm(0.5 * (r["low0"] + r["high0"])))
            else:
                ax.plot(x, pix_to_mm(r["edge_pred_low"]), "c--", lw=1, label="0cm outline moved")
                ax.plot(x, pix_to_mm(r["edge_pred_high"]), "c--", lw=1)
                ax.plot(x, pix_to_mm(r["edge_low1"]), "y-", lw=1, label=f"outline {shot_label}")
                ax.plot(x, pix_to_mm(r["edge_high1"]), "y-", lw=1)
                yc = np.nanmedian(pix_to_mm(0.5 * (r["pred_low"] + r["pred_high"])))
                if r["found"]:
                    ax.plot(r["x_arc"], r["y_arc"], "r.", ms=4, label="extra part")
                    ax.set_xlabel(f"r = {r['radius_mm']:.2f} mm")
            ax.set_xlim(x[0] - 8, x[-1] + 8)
            ax.set_ylim(yc - 40, yc + 40)
            ax.set_title(f"{name} bone - {title}", fontsize=9)
            if j == 0:
                ax.legend(fontsize=6, loc="upper left")
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__))
    raw = os.path.join(here, "..", "raw")
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", default=os.path.join(raw, "xray_image_0cm.dat"))
    ap.add_argument("--shot", default=os.path.join(raw, "xray_image_-20cm.dat"))
    args = ap.parse_args()

    y_src = source_y_from_name(args.shot)
    shot_label = f"{y_src / 10:+.0f}cm{shot_suffix(args.shot)}"
    rel0 = load_relative(args.ref)
    rel1 = load_relative(args.shot)

    boxes0 = find_bones(rel0)
    boxes1 = find_bones(rel1)
    results = {name: analyse_bone(name, rel0, rel1, b0, b1, y_src)
               for name, b0, b1 in zip(BONE_NAMES, boxes0, boxes1)}

    # Save outlines
    f0 = os.path.join(here, "bone_outline_0cm.csv")
    f1 = os.path.join(here, f"outline_{shot_label}.csv")
    save_outline(f0, {n: (r["cols"], r["edge_low0"], r["edge_high0"]) for n, r in results.items()})
    save_outline(f1, {n: (r["cols"], r["edge_low1"], r["edge_high1"]) for n, r in results.items()})

    # Report
    M_exp = magnification(Z_BONE)
    print(f"Gun moved 0 -> {y_src:+.0f} mm. Expected bone shift (z = {Z_BONE:.0f} mm): "
          f"{y_src * (1 - M_exp):+.1f} mm\n")
    print(f"{'bone':>7} {'shift':>8} {'stretch':>8} {'M_bone':>7} {'z_bone':>7} | "
          f"{'side':>6} {'pixels':>6} {'width':>7} {'R_det':>7} {'x_t':>6} "
          f"{'r':>7}")
    for name, r in results.items():
        line = (f"{name:>7} {r['bone_shift_mm']:7.1f}mm {r['bone_stretch_mm']:6.1f}mm "
                f"{r['M_bone']:7.3f} {r['z_bone']:7.1f} | ")
        if r["found"]:
            line += (f"{r['side']:>6} {len(r['run']):6d} {r['width_det_mm']:5.1f}mm "
                     f"{r['R_det']:5.2f}mm {r['x_tumour']:6.1f} "
                     f"{r['radius_mm']:5.2f}mm")
        else:
            line += "no extra part found"
        print(line)

    out_png = os.path.join(here, f"tumour_from_{shot_label}.png")
    plot(results, rel0, rel1, shot_label, out_png)
    print(f"\nSaved {f0}\nSaved {f1}\nSaved {out_png}")
