# MAT267 CAS — TI-84 Plus CE

Native CAS add-in for the TI-84 Plus CE.
Outputs a `.8xp` program file you transfer with TI Connect CE.

---

## Commands

| Key | Operation |
|-----|-----------|
| `1` | Curl & Divergence — curl F and div F at a point |
| `2` | Surface Area — ∬‖rᵤ×rᵥ‖ du dv |
| `3` | Surface Integral — ∬ f dS |
| `4` | Flux Integral — ∬ F·dS (choose orientation) |
| `5` | Volume — between two surfaces in polar coords |
| `6` | Triple Integral — Cartesian / Cylindrical / Spherical |
| `7` | Tangent Plane — z = f(x₀,y₀) + fₓ(x−x₀) + f_y(y−y₀) |

---

## Key layout on the calc

| Soft key | TI-84 key | Action |
|----------|-----------|--------|
| DEL      | Y=        | Backspace |
| CLR      | WINDOW    | Clear field |
| PI       | ZOOM      | Insert `pi` |
| NEG      | TRACE     | Toggle − sign |
| CALC     | GRAPH     | Compute now |
| QUIT     | CLEAR     | Back to menu |

Number keys 1–7 jump directly to any operation from the main menu.
Arrow keys navigate fields. ENTER advances to the next field.
X,T,θ,n key inserts `x`.

---

## How to build (GitHub Actions — easiest)

1. Create a free account at github.com
2. Create a new **public** repository called `mat267ti`
3. Upload all files from this folder (keep the folder structure)
4. Click **Actions** tab → workflow runs automatically
5. When the green checkmark appears, click into the run
6. Scroll to **Artifacts** → download **MAT267TI**
7. Unzip — you get `mat267ti.8xp`

---

## Install onto the calculator

1. Install **TI Connect CE** (free, from education.ti.com)
2. Connect your TI-84 Plus CE via USB
3. Open TI Connect CE → click **Send to Calculator**
4. Select `mat267ti.8xp` → send
5. On the calc: `PRGM` → find `MAT267TI` → `ENTER` to run

Or drag-and-drop the `.8xp` into TI Connect CE's device view.

---

## Expression syntax

| What you want | Type |
|---------------|------|
| π             | `pi` or `p` |
| x²            | `x^2` or press `x²` key |
| sin(x)        | `sin(x)` |
| e^x           | `exp(x)` |
| √x            | `sqrt(x)` |
| ln(x)         | `ln(x)` |
| multiply      | `*` |

Variables available: `x y z` (also `u v`, `r theta`, `rho phi`)

---

## File structure

```
mat267ti/
  CMakeLists.txt
  .github/workflows/build.yml
  src/
    cas.h       — shared types and helpers
    main.c      — entry point and main loop
    eval.c      — expression parser
    fields.c    — input field definitions
    compute.c   — all 7 numerical routines
    ui.c        — drawing and key handling
```
