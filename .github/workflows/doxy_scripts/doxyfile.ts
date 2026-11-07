import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { libraryVersions, type LibraryVersions } from './metadata_version.ts';

export const defaultDescription = 'C++ Toolkit for software engineering';
const defaultRepository = 'JeongHan-Bae/JH-Toolkit';

export function repositoryName(value: string | undefined): string {
  if (!value) return defaultRepository;
  if (!value.startsWith('http')) return value;
  const url = new URL(value);
  if (url.hostname !== 'github.com') throw new Error('GITHUB_REPOSITORY must identify a GitHub repository');
  return url.pathname.replace(/^\/+|\/+$/g, '');
}

export async function repositoryDescription(repository: string, token?: string): Promise<string> {
  try {
    const response = await fetch(`https://api.github.com/repos/${repository}`, {
      headers: {
        Accept: 'application/vnd.github+json',
        ...(token ? { Authorization: `Bearer ${token}` } : {}),
      },
    });
    if (!response.ok) return defaultDescription;
    const data: unknown = await response.json();
    const description = (data as { description?: unknown }).description;
    return typeof description === 'string' && description.trim() ? description.trim() : defaultDescription;
  } catch {
    return defaultDescription;
  }
}

export function renderDoxyfile(template: string, settings: Record<string, string>): string {
  const replacements: Record<string, string> = {
    __VERSION__: settings.version,
    __DESCRIPTION__: settings.description,
    __GENERATE_HTML__: settings.html,
    __GENERATE_XML__: settings.xml,
    __SOURCE_ROOTS__: settings.sourceRoots,
  };
  for (const [marker, value] of Object.entries(replacements)) {
    if (!template.includes(marker)) throw new Error(`Doxyfile template is missing ${marker}`);
    template = template.replaceAll(marker, value.replaceAll('|', '\\|'));
  }
  return template;
}

export function renderPage(template: string, versions: LibraryVersions): string {
  return template
    .replaceAll('__TOOLKIT_VERSION__', `v${versions['jh-toolkit']}`)
    .replaceAll('__CUCUMBER_VERSION__', `v${versions['jh-simple-cucumber']}`)
    .replaceAll('__TINY_TEST_VERSION__', `v${versions['jh-tiny-test']}`);
}

export async function generateDoxyfile(root: string, options: { html: boolean; xml: boolean; repository?: string; token?: string }): Promise<string> {
  const versions = libraryVersions(root);
  const version = `v${versions['jh-toolkit']}`;
  const repository = repositoryName(options.repository);
  const description = await repositoryDescription(repository, options.token);
  const template = readFileSync(join(root, '.github/workflows/doxy_scripts/settings/Doxyfile'), 'utf8');
  mkdirSync(join(root, '.doxy-build'), { recursive: true });
  const pageRoot = join(root, '.doxy-build', 'pages');
  mkdirSync(pageRoot, { recursive: true });
  for (const name of ['mainpage.md', 'toolkit_library.md', 'cucumber_library.md', 'tiny_test_library.md']) {
    const source = readFileSync(join(root, '.github/workflows/doxy_scripts/settings', name), 'utf8');
    writeFileSync(join(pageRoot, name), renderPage(source, versions));
  }
  const footer = readFileSync(join(root, '.github/workflows/doxy_scripts/settings/footer.html'), 'utf8');
  writeFileSync(join(root, '.doxy-build/footer.html'), renderPage(footer, versions));
  writeFileSync(join(root, '.doxy-build/Doxyfile'), renderDoxyfile(template, {
    version, description, html: options.html ? 'YES' : 'NO', xml: options.xml ? 'YES' : 'NO',
    sourceRoots: '.doxy-build/source/include/jh .doxy-build/source/simple-cucumber/include/jh/test/cucumber .doxy-build/source/simple-cucumber/gherkin/include/jh/test/cucumber .doxy-build/source/tiny-test/include/jh/test/tiny_test',
  }));
  console.log(`Doxyfile generated.\nRepo: ${repository}\nVersions: ${JSON.stringify(versions)}\nDescription: ${description}\nGENERATE_HTML: ${options.html ? 'YES' : 'NO'}\nGENERATE_XML: ${options.xml ? 'YES' : 'NO'}`);
  return version;
}
