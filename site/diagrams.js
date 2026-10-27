import mermaid from 'mermaid';
mermaid.initialize({ startOnLoad: false, securityLevel: 'strict', theme: 'neutral',
  fontFamily: 'system-ui, sans-serif', flowchart: { useMaxWidth: true, htmlLabels: false } });
for (const node of document.querySelectorAll('.mermaid')) {
  const source = node.textContent;
  try {
    await mermaid.run({ nodes: [node] });
  } catch {
    node.className = 'diagram-source';
    node.textContent = source;
    const message = document.createElement('p');
    message.textContent = 'This diagram could not be rendered. Its source is shown below.';
    node.before(message);
  }
}
for (const figure of document.querySelectorAll('.diagram')) {
  const svg = figure.querySelector('svg');
  if (!svg) continue;
  let scale = 1;
  const controls = document.createElement('div');
  controls.className = 'diagram-controls';
  const status = document.createElement('span');
  status.textContent = '100%';
  status.setAttribute('aria-live', 'polite');
  for (const [label, action] of [['Zoom out', -0.25], ['Reset', 0], ['Zoom in', 0.25]]) {
    const button = document.createElement('button');
    button.textContent = label;
    button.addEventListener('click', () => {
      scale = action === 0 ? 1 : Math.max(0.5, Math.min(3, scale + action));
      svg.style.maxWidth = 'none';
      svg.style.width = `${scale * 100}%`;
      svg.style.height = 'auto';
      status.textContent = `${Math.round(scale * 100)}%`;
    });
    controls.append(button);
  }
  controls.append(status);
  figure.prepend(controls);
}
