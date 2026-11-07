import { appendFileSync } from 'node:fs';

export function publish(lines: string[], warnings: string[]): void {
  const summary = lines.join('\n') + '\n';
  if (process.env.GITHUB_STEP_SUMMARY) appendFileSync(process.env.GITHUB_STEP_SUMMARY, summary);
  else process.stdout.write(summary);
  for (const warning of warnings) console.log(`::warning::${warning}`);
}
