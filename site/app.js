import './interactive.js';

document.documentElement.classList.add('js');
const navigation = document.querySelector('#navigation');
const navToggle = document.querySelector('#nav-toggle');
navToggle.addEventListener('click', () => {
  const open = navToggle.getAttribute('aria-expanded') !== 'true';
  navToggle.setAttribute('aria-expanded', String(open));
  navigation.classList.toggle('nav-open', open);
});
navigation.addEventListener('click', event => {
  if (event.target.closest('a')) {
    navigation.classList.remove('nav-open');
    navToggle.setAttribute('aria-expanded', 'false');
  }
});

document.querySelectorAll('pre:not(.mermaid)').forEach(pre => {
  const code = pre.querySelector('code');
  if (!code) return;
  const button = document.createElement('button');
  button.className = 'copy-code';
  button.textContent = 'Copy';
  button.setAttribute('aria-label', 'Copy code');
  button.addEventListener('click', async () => {
    try {
      await navigator.clipboard.writeText(code.textContent);
      button.textContent = 'Copied';
    } catch {
      button.textContent = 'Select and copy';
    }
    setTimeout(() => { button.textContent = 'Copy'; }, 1800);
  });
  pre.append(button);
});

const viewer = document.querySelector('#image-viewer');
const fullImage = document.querySelector('#full-image');
function openImage(source, title) {
  fullImage.src = source;
  fullImage.alt = title;
  fullImage.style.width = '100%';
  document.querySelector('#image-title').textContent = title;
  viewer.showModal();
}
document.querySelectorAll('[data-image]').forEach(button => {
  button.addEventListener('click', () => openImage(button.dataset.image, button.dataset.title));
});
document.querySelectorAll('.tutorial img').forEach(img => {
  const button = document.createElement('button');
  button.className = 'tutorial-image';
  button.setAttribute('aria-label', `Inspect ${img.alt}`);
  img.replaceWith(button);
  button.append(img);
  button.addEventListener('click', () => openImage(img.src, img.alt));
});
document.querySelector('#close-image').addEventListener('click', () => viewer.close());
viewer.addEventListener('click', event => { if (event.target === viewer) viewer.close(); });
document.querySelectorAll('[data-image-scale]').forEach(button => {
  button.addEventListener('click', () => {
    fullImage.style.width = button.dataset.imageScale === 'fit' ? '100%' : `${fullImage.naturalWidth * Number(button.dataset.imageScale)}px`;
  });
});

if (document.querySelector('.mermaid')) {
  import('./diagrams.js').catch(() => {
    document.querySelectorAll('.diagram').forEach(figure => {
      const message = document.createElement('p');
      message.textContent = 'Diagram renderer unavailable. The diagram source is shown below.';
      figure.prepend(message);
    });
  });
}
