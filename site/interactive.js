/* Browser teaching models only: these widgets do not execute the RISC-V kernel. */
(() => {
  "use strict";
  const pixels = document.getElementById("pixel-explorer");
  const processes = document.getElementById("process-explorer");

  async function pixelExplorer(root) {
    root.classList.add("widget");
    root.innerHTML = `
      <p class="muted">Explore the real 8×8 font from <code>src/font.c</code>.
      This is a diagram, not a kernel running in your browser.</p>
      <div class="widget-controls">
        <label>ASCII character <input id="pixel-character" type="text" value="A"
          maxlength="1" size="2" autocomplete="off" spellcheck="false"
          aria-describedby="pixel-help"></label>
        <label>Ink color <select id="pixel-color">
          <option value="#ffffff">White</option><option value="#ff0000">Red</option>
          <option value="#00ff00">Green</option><option value="#0000ff">Blue</option>
        </select></label>
      </div>
      <p id="pixel-help" class="muted">Use one printable ASCII character (32–126).
      Hover, click or focus a pixel; arrow keys move within the bitmap. Bit 0 is the leftmost pixel.</p>
      <p class="widget-status" aria-live="polite" aria-atomic="true"></p>
      <div class="bitmap-layout">
        <div class="bitmap-grid" role="group" aria-label="Eight by eight font bitmap"></div>
        <ol class="bitmap-bytes" aria-label="Font bytes by row"></ol>
      </div>
      <p class="widget-detail" id="pixel-detail"></p>`;
    const input = root.querySelector("input");
    const select = root.querySelector("select");
    const grid = root.querySelector(".bitmap-grid");
    const rows = root.querySelector(".bitmap-bytes");
    const status = root.querySelector(".widget-status");
    const detail = root.querySelector(".widget-detail");
    let font, glyph, character = "A", active = 0;
    const hex = (n, digits = 2) => "0x" + n.toString(16).padStart(digits, "0");
    const buttons = Array.from({ length: 64 }, (_, index) => {
      const button = document.createElement("button");
      button.type = "button";
      button.className = "bitmap-cell";
      button.tabIndex = index ? -1 : 0;
      button.dataset.index = String(index);
      button.setAttribute("aria-describedby", "pixel-detail");
      button.addEventListener("pointerenter", () => inspect(index));
      button.addEventListener("focus", () => { active = index; inspect(index); });
      button.addEventListener("click", () => { active = index; inspect(index); });
      grid.append(button);
      return button;
    });
    function inspect(index) {
      if (!glyph) return;
      const x = index % 8, y = Math.floor(index / 8), bit = (glyph[y] >> x) & 1;
      const ink = select.value === "#ffffff" ? "0xffffffff" : "0x00" + select.value.slice(1);
      detail.textContent = `Pixel (x=${x}, y=${y}): bit ${x} of row ${y}, byte ${hex(glyph[y])}, is ${bit}. ` +
        `On a cleared black background, with this glyph at screen origin (0, 0), CEP address = 0x80000000 + 4 × (${y} × 1280 + ${x}) = ` +
        `${hex(0x80000000 + 4 * (y * 1280 + x), 8)}; pixel value ${bit ? ink : "0x00000000"}.`;
      buttons.forEach((button, i) => { button.tabIndex = i === active ? 0 : -1; });
    }
    function render() {
      const code = input.value.codePointAt(0);
      if (!Number.isInteger(code) || code < 32 || code > 126) {
        input.setAttribute("aria-invalid", "true");
        status.textContent = "Enter one printable ASCII character (32–126).";
        return;
      }
      input.removeAttribute("aria-invalid");
      character = String.fromCharCode(code);
      glyph = font[code] || font[character];
      if (!Array.isArray(glyph) || glyph.length !== 8) {
        status.textContent = "This character is missing from the supplied font data.";
        glyph = null;
        return;
      }
      status.textContent = `Character ${character === " " ? "space" : character}: ASCII ${code} (${hex(code)}). ` +
        `${select.options[select.selectedIndex].text} ink; each character occupies 8 × 8 pixels.`;
      buttons.forEach((button, index) => {
        const x = index % 8, y = Math.floor(index / 8), on = (glyph[y] >> x) & 1;
        button.dataset.on = String(Boolean(on));
        button.style.backgroundColor = on ? select.value : "#000000";
        button.setAttribute("aria-label", `Column ${x}, row ${y}: ${on ? "on" : "off"}`);
      });
      rows.replaceChildren(...glyph.map((byte, y) => {
        const item = document.createElement("li");
        const bits = Array.from({ length: 8 }, (_, x) => (byte >> x) & 1).join("");
        item.textContent = `row ${y}: ${hex(byte)} · ${bits} (left → right)`;
        return item;
      }));
      inspect(active);
    }
    grid.addEventListener("keydown", event => {
      const button = event.target.closest("button");
      if (!button) return;
      const index = Number(button.dataset.index), x = index % 8, y = Math.floor(index / 8);
      const targets = { ArrowLeft: index - (x > 0), ArrowRight: index + (x < 7),
        ArrowUp: index - 8 * (y > 0), ArrowDown: index + 8 * (y < 7),
        Home: event.ctrlKey ? 0 : y * 8, End: event.ctrlKey ? 63 : y * 8 + 7 };
      if (!(event.key in targets)) return;
      event.preventDefault();
      buttons[targets[event.key]].focus();
    });
    input.addEventListener("input", () => { if (font) render(); });
    select.addEventListener("change", () => { if (font) render(); });
    try {
      const response = await fetch("./assets/font.json");
      if (!response.ok) throw new Error(`Font request returned ${response.status}`);
      const data = await response.json();
      font = data.glyphs || data;
      render();
    } catch (error) {
      status.textContent = "Font data could not be loaded. Open the site through HTTP(S), or read src/font.c.";
      buttons.forEach(button => { button.disabled = true; });
    }
  }

  function processExplorer(root) {
    root.classList.add("widget");
    root.innerHTML = `
      <p><strong>Scheduling model</strong> — an interactive explanation, not CPU emulation.
      Four fixed slots share one CPU. Each manual tick represents 50 ms (20 Hz).</p>
      <div class="widget-controls">
        <button type="button" data-action="tick">Timer tick (50 ms)</button>
        <button type="button" data-action="sleep">Sleep current (2 s)</button>
        <button type="button" data-action="finish">Finish current</button>
        <button type="button" data-action="reset">Reset</button>
      </div>
      <p class="widget-status" aria-live="polite" aria-atomic="true"></p>
      <div class="process-diagram" role="group" aria-label="Round-robin process slots"></div>
      <p class="muted">Selection checks PID 0 → 1 → 2 → 3 → 0; WAIT and DIE are skipped.
      Click or focus a card to inspect its context. Inspecting a card does not schedule it.</p>
      <p class="widget-detail" aria-live="polite"></p>
      <p class="muted">Sleep uses whole kernel seconds: deadline = floor(ticks / 20) + 2.
      A request between second boundaries can wake after less than two full seconds.
      This model omits instruction execution, UART time and interrupt handling overhead.</p>`;
    const names = ["idle", "proc1", "proc2", "proc3"];
    const states = ["RUN", "RUNNABLE", "WAIT", "DIE"];
    const diagram = root.querySelector(".process-diagram");
    const status = root.querySelector(".widget-status");
    const detail = root.querySelector(".widget-detail");
    const controls = Object.fromEntries([...root.querySelectorAll("[data-action]")].map(b => [b.dataset.action, b]));
    let tasks, current, ticks, inspected = 0, lastEvent;
    const cards = names.map((name, pid) => {
      if (pid) {
        const arrow = document.createElement("span");
        arrow.className = "process-arrow";
        arrow.setAttribute("aria-hidden", "true");
        arrow.textContent = "→";
        diagram.append(arrow);
      }
      const card = document.createElement("button");
      card.type = "button";
      card.className = "process-node";
      card.addEventListener("click", () => { inspected = pid; showContext(); });
      card.addEventListener("focus", () => { inspected = pid; showContext(); });
      diagram.append(card);
      return card;
    });
    function showContext() {
      const task = tasks[inspected];
      cards.forEach((card, pid) => card.setAttribute("aria-pressed", String(pid === inspected)));
      detail.textContent = `PID ${inspected} (${names[inspected]}), ${states[task.state]}: ` +
        (inspected === 0 ? "idle begins on the boot stack; " : "this task has a separate reserved stack; ") +
        "ctx_sw saves ra, sp, gp, tp, s0–s11, mepc and mstatus. sp points to its saved stack; ra and mepc retain return state. " +
        "Temporary and argument registers from an interrupt are saved on the stack by traitant.S. " +
        (task.state === 2 ? `Wake deadline: kernel second ${task.wake}. ` : "") +
        (task.state === 3 ? "This slot is not reused by the current kernel. " : "") +
        "The diagram does not invent register values or stack addresses.";
    }
    function render() {
      cards.forEach((card, pid) => {
        card.dataset.state = String(tasks[pid].state);
        card.replaceChildren(document.createTextNode(`PID ${pid} · ${names[pid]} `));
        const badge = document.createElement("span");
        badge.className = "state-badge";
        badge.textContent = `${states[tasks[pid].state]} (${tasks[pid].state})`;
        card.append(badge);
      });
      controls.sleep.disabled = current === 0;
      controls.finish.disabled = current === 0;
      status.textContent = `Time ${(ticks / 20).toFixed(2)} s · ${ticks} ticks · kernel second ${Math.floor(ticks / 20)} · ` +
        `running PID ${current} (${names[current]}). ${lastEvent}`;
      showContext();
    }
    function schedule() {
      const seconds = Math.floor(ticks / 20);
      tasks.forEach(task => { if (task.state === 2 && seconds >= task.wake) task.state = 1; });
      if (tasks[current].state === 0) tasks[current].state = 1;
      for (let distance = 1; distance <= tasks.length; ++distance) {
        const next = (current + distance) % tasks.length;
        if (tasks[next].state === 1) { current = next; tasks[next].state = 0; break; }
      }
    }
    function reset() {
      tasks = names.map((_, pid) => ({ state: pid ? 1 : 0, wake: 0 }));
      current = ticks = inspected = 0;
      lastEvent = "idle is running; three tasks are ready.";
      render();
    }
    controls.tick.addEventListener("click", () => { ++ticks; schedule(); lastEvent = "Timer interrupt: wake due tasks, then select the next ready slot."; render(); });
    controls.sleep.addEventListener("click", () => {
      if (!current) return;
      tasks[current].state = 2; tasks[current].wake = Math.floor(ticks / 20) + 2;
      lastEvent = `${names[current]} requested sleep until kernel second ${tasks[current].wake}.`;
      schedule(); render();
    });
    controls.finish.addEventListener("click", () => {
      if (!current) return;
      tasks[current].state = 3; lastEvent = `${names[current]} finished and will not be scheduled again.`;
      schedule(); render();
    });
    controls.reset.addEventListener("click", reset);
    reset();
  }
  if (pixels) pixelExplorer(pixels);
  if (processes) processExplorer(processes);
})();
