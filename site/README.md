# Tutorial site

The site contains the home page, nine concept chapters, an examples index and
six standalone example pages. Content comes from `docs/*.md`, `examples/README.md`
and each example's own README. The font and scheduler widgets are explanatory
models; the kernel programs run in QEMU.

Each example page links its local source files to their actual GitHub folder.
Links between guides stay on the website. Central captures are copied from
`docs/evidence/` into `assets/evidence/`; each example's captures, UART log and
report are copied into `assets/examples/NN-name/evidence/`.

With Python 3 and Node.js 20 or newer:

```sh
python3 -m pip install -r site/requirements.txt
npm ci --prefix site
npm run build --prefix site
python3 -m http.server --directory build/site 8000
```

Open `http://localhost:8000`. The `Tutorial site` workflow builds and publishes
the same files to GitHub Pages. Diagram scripts are bundled locally; the site
does not fetch scripts or fonts from a CDN.

Edit the Markdown tutorials for content, `build.py` for the page structure,
`style.css` for layout, and `interactive.js` for the two explanatory widgets.
The builder resolves relative links from the Markdown file that contains them,
so source and image links work in both GitHub READMEs and generated pages.

Before building after a kernel or example change, refresh the actual captures:

```sh
sh scripts/course-env.sh python3 scripts/capture_evidence.py \
  --output docs/evidence --update-examples
```

All six example evidence folders must be present when building the website.
The home gallery, image viewer, Mermaid diagrams and mobile navigation use the
same locally bundled assets on chapter and example pages.
