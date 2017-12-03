import * as fs from "fs";
import * as path from "path";

export function removeFiles(filepath: string): void {
  if (!fs.existsSync(filepath)) {
    return;
  }

  const remaining = [filepath];
  while (true) {
    const target = remaining.pop();
    if (!target) {
      break
    }

    if (fs.statSync(target).isDirectory) {
      const innerFiles = fs.readdirSync(target).map((name) => path.join(target, name));
      if (innerFiles.length === 0) {
        fs.rmdirSync(target);
      } else {
        remaining.concat(innerFiles);
      }
    } else {
      // A normal file.
      fs.unlinkSync(target);
    }
  }
}

export function makedirs(dir: string): void {
  const dirs = path.resolve(dir).split("/");
  let dirpath = "/";

  for (const relpath of dirs) {
    dirpath = path.join(dirpath, relpath);

    if (!fs.existsSync(dirpath)) {
      fs.mkdirSync(dirpath);
    }
  }
}
