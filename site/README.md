# Tutorial site

The site is generated from `docs/*.md` and actual captures in `docs/evidence/`.
It is separate from the kernel build. Its interactive widgets explain the font
and scheduler; they do not run the kernel in the browser.

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
Use the kernel capture script to regenerate screenshots after a demo change.
