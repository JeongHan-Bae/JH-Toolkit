import { readFileSync } from 'node:fs';

export type BadgeConfig = {
  file: string;
  schemaVersion: number;
  label: string;
  labelColor: string;
  namedLogo: string;
  color: string;
  style: string;
};
export type ComponentConfig = { dir: string; kind: 'toolkit' | 'framework'; badge: BadgeConfig };
export type MetadataConfig = {
  project: { description: string };
  components: ComponentConfig[];
};

/** Load the human-maintained metadata that cannot be inferred from CMake. */
export function loadMetadataConfig(): MetadataConfig {
  const config = JSON.parse(readFileSync(new URL('./metadata.config.json', import.meta.url), 'utf8')) as MetadataConfig;
  if (typeof config.project?.description !== 'string' || !config.project.description.trim() ||
      config.components?.length !== 3 || config.components[0]?.dir !== '.')
    throw new Error('metadata.config.json needs a project description and three components, starting with the toolkit');
  const dirs = new Set<string>();
  const files = new Set<string>();
  for (const component of config.components) {
    if (!['toolkit', 'framework'].includes(component.kind) || typeof component.dir !== 'string' ||
        !component.dir || component.dir.includes('..') || dirs.has(component.dir))
      throw new Error(`Invalid or duplicate component directory '${component.dir}' in metadata.config.json`);
    dirs.add(component.dir);
    const badge = component.badge;
    if (!badge || !/^[a-z0-9_]+\.json$/.test(badge.file) || files.has(badge.file) || 'message' in badge ||
        !Number.isInteger(badge.schemaVersion) ||
        ['label', 'labelColor', 'namedLogo', 'color', 'style'].some(key =>
          typeof badge[key as keyof BadgeConfig] !== 'string' || !badge[key as keyof BadgeConfig]))
      throw new Error(`Invalid badge configuration for '${component.dir}'`);
    files.add(badge.file);
  }
  if (config.components[0].kind !== 'toolkit')
    throw new Error('metadata.config.json must start with the toolkit component');
  return config;
}
