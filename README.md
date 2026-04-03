This repository contains a modified version of the Irrlicht Engine SDK source code.

It also mirrors the original source code from [https://sourceforge.net/projects/irrlicht/](https://sourceforge.net/projects/irrlicht/), the official repository of the Irrlicht Engine SDK.

The mirroring is done manually using git-svn. The URL-like text in commit messages is used to track the original upstream revisions, but these links are not directly accessible. You must use `https://sourceforge.net/p/irrlicht/code/<revision number>/` to view the original commit. The revision number is the number after `r` in the commit message.

-----

## Branches

- `trunk` - **Mirror** of https://sourceforge.net/p/irrlicht/code/HEAD/tree/trunk/ . Also known as "upstream", the version code is `1.9`. This branch will never be modified directly.
- `1.8` - **Mirror** of https://sourceforge.net/p/irrlicht/code/HEAD/tree/branches/releases/1.8/ . This branch will never be modified directly.
- `mercury` - ~~Some modifications to the original source code, which are not yet merged into the upstream. Used by the [YGOPro](https://github.com/Fluorohydride/ygopro) project.~~ Current status TBD. It will be renamed to `master` when YGOPro migrates to 1.9.
- `patch-*` - The branches for patches, new features and bug fixes. These branches should be based on `master` or `trunk`. Will be merged into `master` or upstream after testing.
- `dev` - The development branch, which may contain unstable code. Built from `master` and patch branches. Used for testing patches before they are merged into `master` or upstream. Will **never** be merged back to `master`.
- `master` - The current `master` is manually maintained by us, based on the 1.8.5 version. It contains some modifications to the original source code, which are not merged into the upstream. Used by the [YGOPro](https://github.com/Fluorohydride/ygopro) project now. It will be renamed and archived when YGOPro migrates to 1.9.