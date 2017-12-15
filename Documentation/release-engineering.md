---
title: Release Engineering
---

Checklist
---------
- [ ] Runtime: Update version in package.json and `yarn publish`.
- [ ] Supervisor: Update `version` and `dependencies` in package.json and `yarn publish`.
- [ ] SDK: Update `version` and `dependencies` in package.json and `yarn publish`.
- [ ] Build MakeStack Linux, installer, plugins by `./tools/build-release-packages`.
- [ ] Update `changelog.md`.
- [ ] `git tag`
- [ ] `git push --tags`
- [ ] [Create a Release on GitHub](https://github.com/seiyanuta/makestack/releases/new).
