# CITS3007Project

Project for CITS3007

## Formatting

### Formatting C Files

- We are using
  [Uncrustify](https://github.com/uncrustify/uncrustify/tree/master) for
  formatting - `uncrustify.cfg` is located in
  `/.github/workflows/uncrustify.cfg`
  ([here](/.github/workflows/uncrustify.cfg)) - use this file when
  setting up uncrustify (if you use VSCode you may find
  [this](https://marketplace.visualstudio.com/items?itemName=zachflower.uncrustify)
  useful).

- I have aliased this in my shell to make my life easier - y'all are
  more than welcome to use it, or setup your own workflow around
  uncrustify, I don't mind lol

```nushell
alias uncrust='find . \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -exec uncrustify -c ~/.uncrustify.cfg --no-backup {} +'
```

- I have setup a Github action that automatically checks to see if there
  is any code that hasn't been formatted with uncrustify when doing a PR
  \- you can either download the zip file it provides on failure which
  has a git patch you can apply, or (tbh, probably easier) you can just
  run uncrustify on your conmputer and commit+push changes.

- If you find that non `*.c` or `*.h` files are getting picked up but
  the action (it _shouldn't_, but I found it liked to pickup PDF files
  for some reason) let me (Seb) know, and I'll add it to the ignore
  file/directory list.

- If your code isn't formatted, you won't be able to merge it (soz)

- However! If you look below (and you setup pre-commit), you should get
  a warning that your code hasn't been formatted yet (your commit will
  fail), telling you to run uncrustify

### Formatting Markdown files

- Our report needs to match CommonMark spec, and so we also have a
  formatter for that (ikr)
- this is a lil more janky - the main thing to be aware of is text
  wrapping - we shouldn't have lines over 72 chars in length - ngl, we
  may just say stuff it and convert markdown into a pdf when we submit
  it if we get sick of this
- that said, prettier is setup for formatting markdown files, and a
  linter (markdownlint) is setup as an action for any new changes, so
  it'll tell us if something is off
- command to run prettier is `npx prettier . --write` if you installed
  it with npm - if you have a other prefered node package manager thats
  chill, just add any files it creates to `.gitignore`
- anyway, markdown is future issue, mostly here as an FYI
- Also, unlike C formatting, you can merge even if markdown linting
  fails

## Linting

### Linting C Files

#### Pre-Commit

**NOTE**: This repo makes _heavy_ use of
[pre-commits](https://pre-commit.com/) (espically
[this](https://github.com/pocc/pre-commit-hooks) hook lmao) - please
install pre-commit. You will also need to install:

- [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)
- [oclint](https://oclint.org/) (Note to self that i still need to set
  this one up lmao)
- [uncrustify](https://github.com/uncrustify/uncrustify/tree/master)
- [cppcheck](https://cppcheck.sourceforge.io/)
- [cpplint](https://github.com/cpplint/cpplint)
- (possibly) [bear](https://github.com/rizsotto/Bear)

If things aren't working, let me know/if you need a hand installing
stuff (tbh the most annoying will probably be oclint)

If you _need_ to skip running pre-commit hooks for some reason, use
`--no-verify` (eg, `git commit --no-verify`)

#### Github Actions

- I want to add clang-tidy
  [action](https://github.com/marketplace/actions/clang-tidy-review) at
  some point in the future

#### Related Info

Currently `src` just has a few of my lab files, and a super simple make
file - if anyone is a Makefile enjoyer feel free to make a better one
lol A few of the pre-commit hooks require a `compile_commands.json`
which is why we use bear lol
