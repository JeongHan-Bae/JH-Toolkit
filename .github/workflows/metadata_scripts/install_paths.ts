export type InstallPaths = { files: string[]; directories: string[] };

/** Group a complete current directory selection while keeping partial selections as files. */
export function compactInstallPaths(allFiles: string[], selectedFiles: string[], directDirectories: string[]): InstallPaths {
  const all = new Set(allFiles);
  const selected = new Set(selectedFiles);
  for (const file of selected)
    if (!all.has(file)) throw new Error(`Selected install file is missing from source tree: ${file}`);

  const directories = new Set(directDirectories.map(dir => dir.endsWith('/') ? dir : dir + '/'));
  const covered = (path: string) => [...directories].some(dir => path.startsWith(dir));
  const candidates = new Set<string>();
  for (const file of all) {
    const parts = file.split('/');
    for (let index = 1; index < parts.length; index++)
      candidates.add(parts.slice(0, index).join('/') + '/');
  }
  const ordered = [...candidates].sort((a, b) => a.split('/').length - b.split('/').length || a.localeCompare(b));
  for (const directory of ordered) {
    if (covered(directory)) continue;
    const contents = [...all].filter(file => file.startsWith(directory));
    if (contents.length > 1 && contents.every(file => selected.has(file))) directories.add(directory);
  }
  return {
    files: [...selected].filter(file => !covered(file)).sort(),
    directories: [...directories].filter(dir => ![...directories].some(other => other !== dir && dir.startsWith(other))).sort(),
  };
}
