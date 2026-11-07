import { Parser, Language } from 'web-tree-sitter';
import { getWasmPath } from 'tree-sitter-wasm';
import type { CommentPort } from './ports.ts';

export async function cppComments(): Promise<CommentPort> {
  await Parser.init();
  const parser = new Parser();
  parser.setLanguage(await Language.load(getWasmPath('cpp')));
  return {
    hasCopyright(source) {
      const tree = parser.parse(source);
      if (!tree) throw new Error('Could not parse C++ header');
      const cursor = tree.walk();
      do {
        if (cursor.nodeType === 'comment' && cursor.nodeText.includes('@copyright')) return true;
      } while (cursor.gotoFirstChild() || (function next(): boolean {
        while (!cursor.gotoNextSibling()) {
          if (!cursor.gotoParent()) return false;
        }
        return true;
      })());
      return false;
    },
  };
}
