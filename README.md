# CITS3007Project

Project for CITS3007

### Formatting

#### C Files
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
alias uncrust='find . \( -name "*.cpp" -o -name "*.c" -o -name "*.h" \) -exec uncrustify -c ~/.uncrustify.cfg --no-backup {} +
```

- I have setup a GitHub action that automatically checks to see if there
  is any code that hasn't been formatted with uncrustify when doing a PR
  \- you can either download the zip file it provides on failure which
  has a git patch you can apply, or (tbh, probably easier) you can just
  run uncrustify on your conmputer and commit+push changes.

- If you find that non `*.c` or `*.h` files are getting picked up but
  the action (it _shouldn't_, but I found it liked to pickup PDF files
  for some reason) let me (Seb) know, and I'll add it to the ignore
  file/directory list.

- If your code isn't formatted, you won't be able to merge it (soz)
#### Markdown files

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
- Also, unlike C formatting, you can merge even if markdown linting fails 
