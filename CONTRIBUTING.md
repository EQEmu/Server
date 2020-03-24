# EQEMU Contribution Guidelines

## Table of contents

- [Contributing pull requests](#contributing-pull-requests)
- [Code Standards](#code-standards)
- [Communicating with developers](#communicating-with-developers)

## Contributing pull requests

The preferred way to contribute is to fork the repo and submit a pull request on GitHub. If you need help with your changes, you can always post on the forums or try Discord. You can also post unified diffs (git diff should do the trick) on the Server Code Submissions forum, although pull requests will be much quicker and easier on all parties.

The EQEmu Server project is at its core an emulation project.  It aims to offer feature parity with the latest implemented expansion, which at the time of writing is Gates of Discord. As such, be mindful that PRs that do not reflect the state of live at latest implemented expansion or before might not be desired. Please ensure that:

* The PR makes changes that brings the emulator more in line with the state of live at latest implemented expansion
* If the PR addresses Server Admin or Server Ops features, they should be general and apply to most or all servers
* Custom content will not be merged into the Core project
* Do not submit PRs that are very far from completion

When submitted, contributors are prompted to review/comment on the PR with a goal merge window of 7 days. Please be responsive during this period so as to not let PRs get stuck in limbo.

### Be nice to the Git history

Try to make simple PRs that handle one specific topic. It's better to open 3 different PRs that each address a different issue
than one big PR with three commits.

When updating your fork with upstream changes, please use ``git pull --rebase``
to avoid creating "merge commits". Those commits unnecessarily pollute the git
history when coming from PRs.

### Format your commit messages with readability in mind

The way you format your commit messages is quite important to ensure that the
commit history and changelog will be easy to read and understand. A Git commit
message is formatted as a short title (first line) and an extended description
(everything after the first line and an empty separation line).

The short title is the most important part, as it is what will appear in the
`shortlog` changelog (one line per commit, so no description shown) or in the
GitHub interface unless you click the "expand" button. As the name says, try to
keep that first line under 72 characters. It should describe what the commit
does globally, while details would go in the description. Typically, if you
can't keep the title short because you have too much stuff to mention, it means
you should probably split your changes in several commits :)

Here's an example of a well-formatted commit message (note how the extended
description is also manually wrapped at 80 chars for readability):

```text
Prevent French fries carbonization by fixing heat regulation

When using the French fries frying module, Godot would not regulate the heat
and thus bring the oil bath to supercritical liquid conditions, thus causing
unwanted side effects in the physics engine.

By fixing the regulation system via an added binding to the internal feature,
this commit now ensures that Godot will not go past the ebullition temperature
of cooking oil under normal atmospheric conditions.

Fixes #1789, long live the Realm!
```

**Note:** When using the GitHub online editor or its drag-and-drop
feature, *please* edit the commit title to something meaningful. Commits named
"Update my_file.cpp" won't be accepted.

## Code Standards



## Communicating with developers

The EQEmu community is a welcoming community centered mainly around its official communities:

* [The EQEmu Discord Server](https://discord.gg/QHsm7CD)
* [EQEmulator.net Forums](http://www.eqemulator.org/forums/)

Thanks for your interest in contributing!

--- The EQEmu Contributors

This document used the excellent CONTRIBUTING.md file in [The Godot Engine's GitHub repository](https://github.com/godotengine/godot) as a base.