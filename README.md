This repository contains a modified version of the Irrlicht Engine SDK source code.

It also mirrors the original source code from [https://sourceforge.net/projects/irrlicht/](https://sourceforge.net/projects/irrlicht/), the official repository of the Irrlicht Engine SDK.

The mirroring is done manually using git-svn. The URL-like text in commit messages is used to track the original upstream revisions, but these links are not directly accessible. You must use `https://sourceforge.net/p/irrlicht/code/<revision number>/` to view the original commit. The revision number is the number after `r` in the commit message.

-----

## Modifications

- Fixed clipboard on Windows, macOS and Linux. Now it can support Unicode characters (but only double-byte characters on Windows).
- Added Input Method Editor (IME) support on Windows, macOS and Linux, for CJK users.
- Added CI Build support for Windows, macOS and Linux. The test part is still under development (no good graphics support on GitHub Actions runners), but the build part is working.

## Branches

- `trunk` - **Mirror** of https://sourceforge.net/p/irrlicht/code/HEAD/tree/trunk/ . Also known as "upstream", the version code is `1.9`. This branch will never be modified directly.
- `main` - Some modifications to the original source code, which are not yet merged into the upstream. This branch is based on `trunk`. Used by the [YGOPro](https://github.com/Fluorohydride/ygopro) project.
- `patch-*` - The branches for patches, new features and bug fixes. These branches should be based on `main` or `trunk`. Will be merged into `main` or upstream after testing.
- `test` - The development branch, which may contain unstable code. Built from `main` and patch branches. Used for testing patches before they are merged into `main` or upstream. Will **never** be merged back to `main`. **Expect forced pushes and rebases in this branch.**
