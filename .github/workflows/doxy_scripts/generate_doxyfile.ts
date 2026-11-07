import { resolve } from 'node:path';
import { generateDoxyfile } from './doxyfile.ts';

const args = process.argv.slice(2);
await generateDoxyfile(resolve(process.cwd()), {
  html: !args.includes('--no-html'),
  xml: args.includes('--use-xml'),
  repository: process.env.GITHUB_REPOSITORY,
  token: process.env.GITHUB_TOKEN,
});
