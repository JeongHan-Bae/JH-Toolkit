export type FilePort = {
  publicHeaders(): string[];
  exists(path: string): boolean;
  read(path: string): Buffer;
};

export type CommentPort = {
  hasCopyright(source: string): boolean;
};

export type HeaderIssue = {
  path: string;
  nonAscii: boolean;
  badNewline: boolean;
  missingCopyright: boolean;
};
