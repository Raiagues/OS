import { build } from 'esbuild';
import { fileURLToPath } from 'node:url';
const root = fileURLToPath(new URL('../', import.meta.url));
await build({
  absWorkingDir: root,
  entryPoints: ['site/app.js', 'site/diagrams.js'],
  outdir: 'build/site/assets',
  bundle: true, splitting: true, format: 'esm', minify: true,
  target: 'es2022', entryNames: '[name]', chunkNames: 'chunks/[name]-[hash]',
  legalComments: 'linked', logLevel: 'info'
});
