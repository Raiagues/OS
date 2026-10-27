#!/usr/bin/env python3
"""Render the repository tutorials into a static GitHub Pages site."""
import html
import json
from pathlib import Path
import re
import shutil
from markdown_it import MarkdownIt

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/site'
REPO = 'https://github.com/Raiagues/OS'
CHAPTERS = [
    ('01-inventory', 'Kernel overview'), ('02-qemu-riscv', 'QEMU and RISC-V'),
    ('03-boot-kernel', 'Boot and kernel'), ('04-pixels-console', 'Pixels and console'),
    ('05-uart', 'UART output'), ('06-clock-interrupts', 'Clock and interrupts'),
    ('07-processes', 'Processes'), ('08-run-and-verify', 'Run and verify'),
    ('09-development', 'Build it step by step'),
]
CAPTURES = [
    ('pixels', 'Characters, colors and wrapping', 'All printable ASCII characters in four static colors, followed by a long paragraph.', 'pixels'),
    ('console', 'Console controls', 'Newline, tab, carriage return, backspace and clear.', 'console'),
    ('scroll', 'Scrolling', '100 lines written; the oldest lines leave the screen.', 'scroll'),
    ('uart', 'UART output', 'Compare the display with the bytes sent through the serial device.', 'uart'),
    ('clock', 'Clock', 'The timer running with just the idle task.', 'clock'),
    ('processes-start', 'Processes sleeping', 'Three tasks waiting while idle runs.', 'processes'),
    ('processes-end', 'Processes finished', 'All three tasks have terminated; idle and the clock continue.', 'processes'),
]


def page(title, content, current='index'):
    links = ''.join(f'<a href="{slug}.html"'+(' aria-current="page"' if slug == current else '')+f'><span>{i:02d}</span>{label}</a>'
                    for i, (slug, label) in enumerate(CHAPTERS, 1))
    return f'''<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)} · RISC-V OS</title><meta name="description" content="A small RISC-V kernel: source-based tutorials, real QEMU captures and interactive diagrams.">
<link rel="stylesheet" href="assets/style.css"><script type="module" src="assets/app.js"></script></head>
<body><a class="skip-link" href="#content">Skip to content</a>
<header class="topbar"><a class="brand" href="index.html"><span class="chip">RV64</span> OS <span class="brand-note">/ kernel notebook</span></a>
<div class="top-links"><a href="{REPO}">Source on GitHub ↗</a><button id="nav-toggle" aria-expanded="false" aria-controls="navigation">Chapters</button></div></header>
<div class="layout"><aside id="navigation"><nav aria-label="Project guide"><a class="nav-home" href="index.html"{' aria-current="page"' if current == 'index' else ''}>Overview</a>
<p class="nav-label">TUTORIALS</p>{links}<p class="nav-label">EXPLORE</p><a href="index.html#captures">QEMU captures</a><a href="index.html#interactive">Interactive diagrams</a></nav>
<div class="sidebar-note">C + RISC-V assembly<br>One hart · Machine mode<br>Framebuffer · UART · CLINT</div></aside>
<main id="content">{content}<footer>RISC-V OS · Emilly Raiane Rodrigues<br><a href="{REPO}">Source code, credits and license notices</a></footer></main></div>
<dialog id="image-viewer" aria-labelledby="image-title"><div class="viewer-controls"><strong id="image-title">QEMU capture</strong><button data-image-scale="fit">Fit</button><button data-image-scale="1">100%</button><button data-image-scale="2">200%</button><button id="close-image" aria-label="Close image">Close ×</button></div><div class="image-pan"><img id="full-image" alt=""></div></dialog>
</body></html>'''


def home():
    cards = ''.join(f'''<figure class="capture-card"><button class="capture-open" data-image="assets/evidence/{name}.png" data-title="{title}" aria-label="Inspect {title.lower()}"><img src="assets/evidence/{name}.png" alt="{title}: actual CEP framebuffer" loading="lazy"></button><figcaption><h3>{title}</h3><p>{description}</p><a href="assets/evidence/{log}.txt">Read the UART log ↗</a></figcaption></figure>'''
                    for name, title, description, log in CAPTURES)
    lessons = ''.join(f'<a class="lesson-link" href="{slug}.html"><span>{i:02d}</span><strong>{title}</strong><span aria-hidden="true">→</span></a>'
                      for i, (slug, title) in enumerate(CHAPTERS, 1))
    return f'''<section class="intro"><p class="eyebrow">RISC-V OS · EMILLY RAIANE RODRIGUES</p><h1>A small RISC-V kernel.</h1><p class="lead">Read the code. Run it in QEMU. Follow a character into the framebuffer, and a timer interrupt into the scheduler.</p><p>This project implements a graphical console, UART output, a clock and four kernel tasks on a single RISC-V hart. Explore how each part works, then follow the implementation step by step.</p><div class="actions"><a class="button primary" href="08-run-and-verify.html">Run the project</a><a class="button" href="09-development.html">Build it step by step</a></div></section>
<section class="start-panel"><div><p class="eyebrow">OPEN THE GRAPHICAL CONSOLE</p><h2>Boot the kernel</h2><p>Set up the RISC-V compiler and a QEMU build with the CEP machine using the <a href="08-run-and-verify.html">installation guide</a>. This command then builds the kernel and opens its graphical window.</p></div><pre><code>./run.sh</code></pre></section>
<section id="captures"><div class="section-heading"><p class="eyebrow">REAL OUTPUT</p><h2>Inside the QEMU window</h2><p>These images were read from the kernel's actual 1280 × 720 framebuffer. Select a capture to inspect it at full size; the small previews show its upper portion.</p></div><div class="capture-grid">{cards}</div><p class="source-note"><a href="assets/evidence/report.json">Capture report and source hashes</a> · <a href="08-run-and-verify.html#capture-the-real-framebuffer">Generate the same evidence</a></p></section>
<section id="interactive"><div class="section-heading"><p class="eyebrow">INTERACTIVE DIAGRAMS</p><h2>Explore the mechanisms</h2><p>These are explanatory models in your browser. The kernel itself runs in QEMU using the commands in the tutorials.</p></div><section class="widget"><h3>Eight bytes become a character</h3><p>Choose a character from the kernel's font and inspect the address of each pixel.</p><div id="pixel-explorer"></div><a class="widget-link" href="04-pixels-console.html">Read the console implementation →</a></section><section class="widget"><h3>One hart, four tasks</h3><p>Advance the timer, put a task to sleep or finish it. Follow which task receives the CPU.</p><div id="process-explorer"></div><a class="widget-link" href="07-processes.html">Read the scheduler implementation →</a></section></section>
<section><div class="section-heading"><p class="eyebrow">SOURCE-BASED TUTORIALS</p><h2>From boot to processes</h2><p>Each chapter connects the code, the design choices and a reproducible experiment.</p></div><div class="lesson-list">{lessons}</div></section>'''


def destination(url):
    if url.startswith('evidence/'):
        return 'assets/' + url
    if url == '../README.md' or url == 'README.md':
        return 'index.html'
    if url.startswith('../'):
        return REPO + '/blob/main/' + url[3:]
    if '.md' in url and '://' not in url:
        return url.replace('.md', '.html')
    return url


def chapter(path):
    md = MarkdownIt('commonmark', {'html': True}).enable('table')
    original_fence = md.renderer.rules['fence']
    def fence(tokens, idx, options, env):
        if tokens[idx].info.strip() == 'mermaid':
            return '<div class="diagram"><pre class="mermaid">'+html.escape(tokens[idx].content)+'</pre></div>'
        return original_fence(tokens, idx, options, env)
    md.renderer.rules['fence'] = fence
    tokens = md.parse(path.read_text())
    used = {}
    for i, token in enumerate(tokens):
        if token.type == 'heading_open':
            slug = re.sub(r'[^\w\s-]', '', tokens[i+1].content.lower()).replace(' ', '-')
            number = used.get(slug, 0)
            used[slug] = number + 1
            token.attrSet('id', slug + (f'-{number}' if number else ''))
        for child in token.children or []:
            if child.type == 'link_open':
                child.attrSet('href', destination(child.attrGet('href')))
            if child.type == 'image':
                child.attrSet('src', destination(child.attrGet('src')))
                child.attrSet('loading', 'lazy')
    rendered = md.renderer.render(tokens, md.options, {})
    return '<article class="tutorial">'+rendered+'</article>'


def main():
    if OUT.exists():
        shutil.rmtree(OUT)
    (OUT / 'assets').mkdir(parents=True)
    shutil.copy(ROOT / 'site/style.css', OUT / 'assets/style.css')
    shutil.copytree(ROOT / 'docs/evidence', OUT / 'assets/evidence')
    font = re.findall(r'\{((?:\s*0x[0-9a-fA-F]+\s*,?){8})\}', (ROOT / 'src/font.c').read_text())
    glyphs = {str(i): [int(x, 16) for x in re.findall(r'0x[0-9a-fA-F]+', row)] for i, row in enumerate(font)}
    assert len(glyphs) >= 128, 'Font table extraction failed'
    (OUT / 'assets/font.json').write_text(json.dumps(glyphs))
    (OUT / 'index.html').write_text(page('Overview', home()))
    for i, (slug, title) in enumerate(CHAPTERS):
        content = chapter(ROOT / f'docs/{slug}.md')
        content += '<nav class="chapter-pagination" aria-label="Adjacent chapters">'
        if i:
            content += f'<a href="{CHAPTERS[i-1][0]}.html">← {CHAPTERS[i-1][1]}</a>'
        if i + 1 < len(CHAPTERS):
            content += f'<a href="{CHAPTERS[i+1][0]}.html">{CHAPTERS[i+1][1]} →</a>'
        content += '</nav>'
        (OUT / f'{slug}.html').write_text(page(title, content, slug))
    (OUT / '.nojekyll').touch()
    print(f'Built {len(CHAPTERS) + 1} pages in {OUT}')


if __name__ == '__main__':
    main()
